#pragma once
#include <QQuickRhiItem>
#include <QByteArray>
#include <QPointer>
#include <QVector>
#include <QtQml/qqmlregistration.h>
#include "spectrumtelemetry.h"

class WaterfallItem : public QQuickRhiItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(SpectrumTelemetry *source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(float minDb READ minDb WRITE setMinDb NOTIFY minDbChanged)
    Q_PROPERTY(float maxDb READ maxDb WRITE setMaxDb NOTIFY maxDbChanged)

public:
    static constexpr int kTexW = 2048;     // columns (frequency)
    static constexpr int kTexH = 512;      // rows (history)
    static constexpr float kStoreMin = -160.f;
    static constexpr float kStoreMax = 0.f;

    explicit WaterfallItem(QQuickItem *parent = nullptr);
    QQuickRhiItemRenderer *createRenderer() override;

    SpectrumTelemetry *source() const { return m_source; }
    float minDb() const { return m_minDb; }
    float maxDb() const { return m_maxDb; }

    void setSource(SpectrumTelemetry *s);
    void setMinDb(float v);
    void setMaxDb(float v);

signals:
    void sourceChanged();
    void minDbChanged();
    void maxDbChanged();

private:
    friend class WaterfallRenderer;
    void onFrame(const QVector<float> &dbm);
    void onTuningChanged();

    QPointer<SpectrumTelemetry> m_source;
    QMetaObject::Connection m_frameConn, m_tuneConn;

    QVector<QByteArray> m_rows;     // rows waiting for the render thread
    bool m_clear = false;
    float m_minDb = -140.f;
    float m_maxDb = -40.f;
};