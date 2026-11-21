#include "waterfallitem.h"
#include <rhi/qrhi.h>
#include <QFile>
#include <QMatrix4x4>
#include <QDebug>
#include <algorithm>

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
class WaterfallRenderer : public QQuickRhiItemRenderer
{
public:
    void initialize(QRhiCommandBuffer *cb) override
    {
        if (m_rhi != rhi()) {
            m_pipeline.reset(); m_srb.reset(); m_sampler.reset();
            m_tex.reset(); m_ubuf.reset(); m_vbuf.reset();
            m_rhi = rhi();
        }
        if (m_pipeline)
            return;

        // fullscreen quad: x, y, u, v   (v = 0 at the top)
        static const float quad[] = {
            -1.f,  1.f, 0.f, 0.f,
            -1.f, -1.f, 0.f, 1.f,
            1.f,  1.f, 1.f, 0.f,
            1.f, -1.f, 1.f, 1.f,
        };
        m_vbuf.reset(m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(quad)));
        m_vbuf->create();

        m_ubuf.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 80));
        m_ubuf->create();

        m_tex.reset(m_rhi->newTexture(QRhiTexture::R8,
                                      QSize(WaterfallItem::kTexW, WaterfallItem::kTexH)));
        m_tex->create();

        m_sampler.reset(m_rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear,
                                          QRhiSampler::None,
                                          QRhiSampler::ClampToEdge, QRhiSampler::Repeat));
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
            { QRhiShaderStage::Vertex,   loadShader(":/shaders/waterfall.vert.qsb") },
            { QRhiShaderStage::Fragment, loadShader(":/shaders/waterfall.frag.qsb") }
        });

        QRhiVertexInputLayout layout;
        layout.setBindings({ QRhiVertexInputBinding(quint32(4 * sizeof(float))) });
        layout.setAttributes({
            QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float2, 0),
            QRhiVertexInputAttribute(0, 1, QRhiVertexInputAttribute::Float2, 2 * sizeof(float))
        });
        m_pipeline->setVertexInputLayout(layout);
        m_pipeline->setShaderResourceBindings(m_srb.get());
        m_pipeline->setRenderPassDescriptor(renderTarget()->renderPassDescriptor());
        if (!m_pipeline->create())
            qWarning() << "Waterfall pipeline creation failed";

        QRhiResourceUpdateBatch *u = m_rhi->nextResourceUpdateBatch();
        u->uploadStaticBuffer(m_vbuf.get(), quad);
        cb->resourceUpdate(u);

        m_needClear = true;     // new texture has undefined contents
    }

    // GUI thread is blocked here, so reading the item is safe
    void synchronize(QQuickRhiItem *item) override
    {
        auto *it = static_cast<WaterfallItem *>(item);
        m_minDb = it->m_minDb;
        m_maxDb = it->m_maxDb;
        if (it->m_clear) {
            m_needClear = true;
            it->m_clear = false;
        }
        m_rows = std::move(it->m_rows);
        it->m_rows.clear();
    }

    void render(QRhiCommandBuffer *cb) override
    {
        constexpr int W = WaterfallItem::kTexW;
        constexpr int H = WaterfallItem::kTexH;

        QRhiResourceUpdateBatch *u = m_rhi->nextResourceUpdateBatch();

        if (m_needClear) {
            QRhiTextureSubresourceUploadDescription zero;
            zero.setData(QByteArray(W * H, 0));
            u->uploadTexture(m_tex.get(), QRhiTextureUploadEntry(0, 0, zero));
            m_head = 0;
            m_needClear = false;
        }

        for (const QByteArray &row : std::as_const(m_rows)) {
            QRhiTextureSubresourceUploadDescription sub;
            sub.setData(row);
            sub.setSourceSize(QSize(W, 1));
            sub.setDestinationTopLeft(QPoint(0, m_head));
            u->uploadTexture(m_tex.get(), QRhiTextureUploadEntry(0, 0, sub));
            m_head = (m_head + 1) % H;
        }
        m_rows.clear();

        const int newest = (m_head - 1 + H) % H;
        const float params[4] = { m_minDb, m_maxDb,
                                 (newest + 0.5f) / H,
                                 float(H - 1) / H };

        const QMatrix4x4 corr = m_rhi->clipSpaceCorrMatrix();
        u->updateDynamicBuffer(m_ubuf.get(), 0,  64, corr.constData());
        u->updateDynamicBuffer(m_ubuf.get(), 64, 16, params);

        const QSize sz = renderTarget()->pixelSize();
        cb->beginPass(renderTarget(), QColor(0, 0, 0, 255), { 1.0f, 0 }, u);
        cb->setGraphicsPipeline(m_pipeline.get());
        cb->setViewport(QRhiViewport(0, 0, sz.width(), sz.height()));
        cb->setShaderResources();
        const QRhiCommandBuffer::VertexInput vi(m_vbuf.get(), 0);
        cb->setVertexInput(0, 1, &vi);
        cb->draw(4);
        cb->endPass();
    }

private:
    QRhi *m_rhi = nullptr;
    std::unique_ptr<QRhiBuffer> m_vbuf, m_ubuf;
    std::unique_ptr<QRhiTexture> m_tex;
    std::unique_ptr<QRhiSampler> m_sampler;
    std::unique_ptr<QRhiShaderResourceBindings> m_srb;
    std::unique_ptr<QRhiGraphicsPipeline> m_pipeline;

    QVector<QByteArray> m_rows;
    int m_head = 0;                  // next row to write
    bool m_needClear = true;
    float m_minDb = -140.f, m_maxDb = -40.f;
};

// ---------------------------------------------------------------- Item
WaterfallItem::WaterfallItem(QQuickItem *parent) : QQuickRhiItem(parent) {}

QQuickRhiItemRenderer *WaterfallItem::createRenderer()
{
    return new WaterfallRenderer;
}

void WaterfallItem::setSource(SpectrumTelemetry *s)
{
    if (m_source == s) return;

    QObject::disconnect(m_frameConn);
    QObject::disconnect(m_tuneConn);
    m_source = s;
    if (m_source) {
        m_frameConn = connect(m_source, &SpectrumTelemetry::frameReady,
                              this, &WaterfallItem::onFrame);
        m_tuneConn  = connect(m_source, &SpectrumTelemetry::axisChanged,
                             this, &WaterfallItem::onTuningChanged);
    }
    emit sourceChanged();
}

void WaterfallItem::onTuningChanged()
{
    m_rows.clear();
    m_clear = true;
    update();
}

void WaterfallItem::onFrame(const QVector<float> &dbm)
{
    const int n = dbm.size();
    if (n < 2) return;

    // resample to kTexW columns, keeping the peak of each group
    QByteArray row(kTexW, 0);
    auto *out = reinterpret_cast<quint8 *>(row.data());
    for (int x = 0; x < kTexW; ++x) {
        const int i0 = int(qint64(x) * n / kTexW);
        const int i1 = std::min(n, std::max(i0 + 1, int(qint64(x + 1) * n / kTexW)));
        float m = dbm[i0];
        for (int i = i0 + 1; i < i1; ++i)
            m = std::max(m, dbm[i]);
        const float t = (m - kStoreMin) / (kStoreMax - kStoreMin);
        out[x] = quint8(std::clamp(t * 255.f + 0.5f, 0.f, 255.f));
    }

    m_rows.append(row);
    if (m_rows.size() > kTexH)
        m_rows.remove(0, m_rows.size() - kTexH);
    update();
}

void WaterfallItem::setMinDb(float v)
{
    if (m_minDb == v) return;
    m_minDb = v; emit minDbChanged(); update();
}

void WaterfallItem::setMaxDb(float v)
{
    if (m_maxDb == v) return;
    m_maxDb = v; emit maxDbChanged(); update();
}