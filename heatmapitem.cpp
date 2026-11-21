#include "heatmapitem.h"
#include <rhi/qrhi.h>
#include <QFile>
#include <QMatrix4x4>
#include <QDebug>
#include <cmath>

static QShader loadShader(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open shader" << path;
        return QShader();
    }
    QShader s = QShader::fromSerialized(f.readAll());
    if (!s.isValid())
        qWarning() << "Invalid shader" << path;
    return s;
}

// ---------------------------------------------------------------- Renderer
class HeatmapRenderer : public QQuickRhiItemRenderer
{
public:
    void initialize(QRhiCommandBuffer *cb) override
    {
        constexpr int N = DfTelemetry::kGrid;

        if (m_rhi != rhi()) {
            m_pipeline.reset(); m_srb.reset(); m_sampler.reset();
            m_tex.reset(); m_ubuf.reset(); m_vbuf.reset();
            m_rhi = rhi();
        }
        if (m_pipeline)
            return;

        static const float corners[] = { 0.f, 0.f,  0.f, 1.f,  1.f, 0.f,  1.f, 1.f };
        m_vbuf.reset(m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(corners)));
        m_vbuf->create();

        m_ubuf.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 96));
        m_ubuf->create();

        m_tex.reset(m_rhi->newTexture(QRhiTexture::R8, QSize(N, N)));
        m_tex->create();

        m_sampler.reset(m_rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                          QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge));
        m_sampler->create();

        m_srb.reset(m_rhi->newShaderResourceBindings());
        m_srb->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(
                0,
                QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage,
                m_ubuf.get()),
            QRhiShaderResourceBinding::sampledTexture(
                1, QRhiShaderResourceBinding::FragmentStage, m_tex.get(), m_sampler.get())
        });
        m_srb->create();

        m_pipeline.reset(m_rhi->newGraphicsPipeline());
        m_pipeline->setTopology(QRhiGraphicsPipeline::TriangleStrip);
        m_pipeline->setShaderStages({
            { QRhiShaderStage::Vertex,   loadShader(":/shaders/heatmap.vert.qsb") },
            { QRhiShaderStage::Fragment, loadShader(":/shaders/heatmap.frag.qsb") }
        });

        QRhiVertexInputLayout layout;
        layout.setBindings({ QRhiVertexInputBinding(quint32(2 * sizeof(float))) });
        layout.setAttributes({ QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float2, 0) });
        m_pipeline->setVertexInputLayout(layout);
        m_pipeline->setShaderResourceBindings(m_srb.get());
        m_pipeline->setRenderPassDescriptor(renderTarget()->renderPassDescriptor());
        if (!m_pipeline->create())
            qWarning() << "Heatmap pipeline creation failed";

        QRhiResourceUpdateBatch *u = m_rhi->nextResourceUpdateBatch();
        u->uploadStaticBuffer(m_vbuf.get(), corners);
        cb->resourceUpdate(u);

        if (m_grid.isEmpty())
            m_grid = QByteArray(N * N, '\0');
        m_upload = true;                // new texture has undefined contents
    }

    // GUI thread is blocked here, so reading the item is safe
    void synchronize(QQuickRhiItem *item) override
    {
        auto *it = static_cast<HeatmapItem *>(item);
        const QRectF r = it->m_viewRect;
        const double w = it->width(), h = it->height();

        m_valid = w > 0 && h > 0
                  && std::isfinite(r.x()) && std::isfinite(r.y())
                  && std::isfinite(r.width()) && std::isfinite(r.height())
                  && r.width() > 0 && r.height() > 0;
        if (m_valid) {
            m_rect[0] = float(r.x() / w);     m_rect[1] = float(r.y() / h);
            m_rect[2] = float(r.width() / w); m_rect[3] = float(r.height() / h);
        }
        m_intensity = it->m_intensity;

        if (it->m_dirty) {
            m_grid = it->m_grid;
            it->m_dirty = false;
            m_upload = true;
        }
    }

    void render(QRhiCommandBuffer *cb) override
    {
        constexpr int N = DfTelemetry::kGrid;
        QRhiResourceUpdateBatch *u = m_rhi->nextResourceUpdateBatch();

        if (m_upload && m_grid.size() == N * N) {
            QRhiTextureSubresourceUploadDescription sub;
            sub.setData(m_grid);
            sub.setSourceSize(QSize(N, N));
            u->uploadTexture(m_tex.get(), QRhiTextureUploadEntry(0, 0, sub));
            m_upload = false;
        }

        const QMatrix4x4 corr = m_rhi->clipSpaceCorrMatrix();
        const float params[4] = { m_intensity, 0.f, 0.f, 0.f };
        u->updateDynamicBuffer(m_ubuf.get(), 0,  64, corr.constData());
        u->updateDynamicBuffer(m_ubuf.get(), 64, 16, m_rect);
        u->updateDynamicBuffer(m_ubuf.get(), 80, 16, params);

        const QSize sz = renderTarget()->pixelSize();
        cb->beginPass(renderTarget(), QColor(0, 0, 0, 0), { 1.0f, 0 }, u);   // transparent
        if (m_valid) {
            cb->setGraphicsPipeline(m_pipeline.get());
            cb->setViewport(QRhiViewport(0, 0, sz.width(), sz.height()));
            cb->setShaderResources();
            const QRhiCommandBuffer::VertexInput vi(m_vbuf.get(), 0);
            cb->setVertexInput(0, 1, &vi);
            cb->draw(4);
        }
        cb->endPass();
    }

private:
    QRhi *m_rhi = nullptr;
    std::unique_ptr<QRhiBuffer> m_vbuf, m_ubuf;
    std::unique_ptr<QRhiTexture> m_tex;
    std::unique_ptr<QRhiSampler> m_sampler;
    std::unique_ptr<QRhiShaderResourceBindings> m_srb;
    std::unique_ptr<QRhiGraphicsPipeline> m_pipeline;

    QByteArray m_grid;
    bool m_upload = false;
    bool m_valid = false;
    float m_rect[4] = { 0, 0, 0, 0 };
    float m_intensity = 0.75f;
};

// ---------------------------------------------------------------- Item
HeatmapItem::HeatmapItem(QQuickItem *parent) : QQuickRhiItem(parent)
{
    setAlphaBlending(true);
}

QQuickRhiItemRenderer *HeatmapItem::createRenderer()
{
    return new HeatmapRenderer;
}

void HeatmapItem::setLocator(DfTelemetry *l)
{
    if (m_locator == l) return;

    QObject::disconnect(m_conn);
    m_locator = l;
    if (m_locator) {
        m_conn = connect(m_locator, &DfTelemetry::gridReady, this, &HeatmapItem::onGrid);
        onGrid(m_locator->grid());
    }
    emit locatorChanged();
}

void HeatmapItem::onGrid(const QByteArray &grid)
{
    m_grid = grid;           // implicitly shared, cheap
    m_dirty = true;
    update();
}

void HeatmapItem::setViewRect(const QRectF &r)
{
    if (m_viewRect == r) return;
    m_viewRect = r; emit viewRectChanged(); update();
}

void HeatmapItem::setIntensity(float v)
{
    if (qFuzzyCompare(m_intensity, v)) return;
    m_intensity = v; emit intensityChanged(); update();
}