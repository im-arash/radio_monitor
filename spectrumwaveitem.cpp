#include "spectrumwaveitem.h"
#include <rhi/qrhi.h>
#include <QFile>
#include <QMatrix4x4>
#include <QRandomGenerator>
#include <cmath>
#include <QDebug>

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
class SpectrumRenderer : public QQuickRhiItemRenderer
{
public:
    void initialize(QRhiCommandBuffer *) override
    {
        if (m_rhi != rhi()) {               // new backend / context
            m_pipeline.reset(); m_srb.reset(); m_ubuf.reset(); m_vbuf.reset();
            m_rhi = rhi();
        }
        if (m_pipeline)
            return;

        m_vbuf.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer,
                                      MaxBins * 2 * sizeof(float)));
        m_vbuf->create();

        m_ubuf.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 96));
        m_ubuf->create();

        m_srb.reset(m_rhi->newShaderResourceBindings());
        m_srb->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(
                0,
                QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage,
                m_ubuf.get())
        });
        m_srb->create();

        m_pipeline.reset(m_rhi->newGraphicsPipeline());
        m_pipeline->setTopology(QRhiGraphicsPipeline::LineStrip);
        m_pipeline->setShaderStages({
            { QRhiShaderStage::Vertex,   loadShader(":/shaders/spectrum.vert.qsb") },
            { QRhiShaderStage::Fragment, loadShader(":/shaders/spectrum.frag.qsb") }
        });

        QRhiVertexInputLayout layout;
        layout.setBindings({ QRhiVertexInputBinding(quint32(2 * sizeof(float))) });
        layout.setAttributes({ QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float2, 0) });
        m_pipeline->setVertexInputLayout(layout);

        m_pipeline->setShaderResourceBindings(m_srb.get());
        m_pipeline->setRenderPassDescriptor(renderTarget()->renderPassDescriptor());
        m_pipeline->create();

        if (!m_pipeline->create())
            qWarning() << "Spectrum pipeline creation failed";
    }

    // Runs on the render thread while the GUI thread is blocked -> safe to read the item
    void synchronize(QQuickRhiItem *item) override
    {
        auto *it = static_cast<SpectrumWaveItem *>(item);
        m_minDb = it->m_minDb;
        m_maxDb = it->m_maxDb;
        m_color = it->m_color;
        if (it->m_dataDirty) {
            m_data = it->m_data;
            it->m_dataDirty = false;
            m_uploadData = true;
        }
    }

    void render(QRhiCommandBuffer *cb) override
    {
        QRhiResourceUpdateBatch *u = m_rhi->nextResourceUpdateBatch();

        if (m_uploadData) {
            m_count = qMin<int>(m_data.size(), MaxBins);
            m_verts.resize(m_count * 2);
            for (int i = 0; i < m_count; ++i) {
                m_verts[2 * i]     = m_count > 1 ? float(i) / float(m_count - 1) : 0.f;
                m_verts[2 * i + 1] = m_data[i];
            }
            if (m_count > 0)
                u->updateDynamicBuffer(m_vbuf.get(), 0, m_count * 2 * sizeof(float), m_verts.constData());
            m_uploadData = false;
        }

        const QMatrix4x4 corr = m_rhi->clipSpaceCorrMatrix();
        const float color[4] = { float(m_color.redF()), float(m_color.greenF()),
                                float(m_color.blueF()), 1.f };
        const float range[2] = { m_minDb, m_maxDb };
        u->updateDynamicBuffer(m_ubuf.get(), 0,  64, corr.constData());
        u->updateDynamicBuffer(m_ubuf.get(), 64, 16, color);
        u->updateDynamicBuffer(m_ubuf.get(), 80, 8,  range);

        const QSize sz = renderTarget()->pixelSize();
        cb->beginPass(renderTarget(), QColor(0, 0, 0, 0), { 1.0f, 0 }, u);   // transparent
        if (m_count > 1) {
            cb->setGraphicsPipeline(m_pipeline.get());
            cb->setViewport(QRhiViewport(0, 0, sz.width(), sz.height()));
            cb->setShaderResources();
            const QRhiCommandBuffer::VertexInput vi(m_vbuf.get(), 0);
            cb->setVertexInput(0, 1, &vi);
            cb->draw(m_count);
        }
        cb->endPass();
    }

private:
    static constexpr int MaxBins = 16384;

    QRhi *m_rhi = nullptr;
    std::unique_ptr<QRhiBuffer> m_vbuf, m_ubuf;
    std::unique_ptr<QRhiShaderResourceBindings> m_srb;
    std::unique_ptr<QRhiGraphicsPipeline> m_pipeline;

    QVector<float> m_data, m_verts;
    bool m_uploadData = false;
    int m_count = 0;
    float m_minDb = -140.f, m_maxDb = -40.f;
    QColor m_color;
};

// ---------------------------------------------------------------- Item
SpectrumWaveItem::SpectrumWaveItem(QQuickItem *parent) : QQuickRhiItem(parent)
{
    setAlphaBlending(true);
}

QQuickRhiItemRenderer *SpectrumWaveItem::createRenderer()
{
    return new SpectrumRenderer;
}

void SpectrumWaveItem::setSource(SpectrumTelemetry *s)
{
    if (m_source == s) return;

    QObject::disconnect(m_conn);
    m_source = s;
    if (m_source)
        m_conn = connect(m_source, &SpectrumTelemetry::frameReady,
                         this, &SpectrumWaveItem::onFrame);
    emit sourceChanged();
}

void SpectrumWaveItem::onFrame(const QVector<float> &dbm)
{
    m_data = dbm;            // QVector is implicitly shared, so this is cheap
    m_dataDirty = true;
    update();
}

void SpectrumWaveItem::setMinDb(float v)
{
    if (m_minDb == v) return;
    m_minDb = v; emit minDbChanged(); update();
}

void SpectrumWaveItem::setMaxDb(float v)
{
    if (m_maxDb == v) return;
    m_maxDb = v; emit maxDbChanged(); update();
}

void SpectrumWaveItem::setTraceColor(const QColor &c)
{
    if (m_color == c) return;
    m_color = c; emit traceColorChanged(); update();
}