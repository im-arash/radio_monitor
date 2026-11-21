#pragma once
#include <QQuickRhiItem>
#include <QColor>
#include <QPointer>
#include <QVector>
#include <QtQml/qqmlregistration.h>
#include "spectrumtelemetry.h"

class SpectrumWaveItem : public QQuickRhiItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(SpectrumTelemetry *source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(float minDb READ minDb WRITE setMinDb NOTIFY minDbChanged)
    Q_PROPERTY(float maxDb READ maxDb WRITE setMaxDb NOTIFY maxDbChanged)
    Q_PROPERTY(QColor traceColor READ traceColor WRITE setTraceColor NOTIFY traceColorChanged)

public:
    explicit SpectrumWaveItem(QQuickItem *parent = nullptr);
    QQuickRhiItemRenderer *createRenderer() override;

    SpectrumTelemetry *source() const { return m_source; }
    float minDb() const { return m_minDb; }
    float maxDb() const { return m_maxDb; }
    QColor traceColor() const { return m_color; }

    void setSource(SpectrumTelemetry *s);
    void setMinDb(float v);
    void setMaxDb(float v);
    void setTraceColor(const QColor &c);

signals:
    void sourceChanged();
    void minDbChanged();
    void maxDbChanged();
    void traceColorChanged();

private:
    friend class SpectrumRenderer;
    void onFrame(const QVector<float> &dbm);

    QPointer<SpectrumTelemetry> m_source;
    QMetaObject::Connection m_conn;

    QVector<float> m_data;
    bool m_dataDirty = false;
    float m_minDb = -140.f;
    float m_maxDb = -40.f;
    QColor m_color = QColor(255, 220, 0);
};