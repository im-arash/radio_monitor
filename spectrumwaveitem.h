#pragma once

#include <QQuickPaintedItem>
#include <QColor>
#include <QPointer>
#include <QVector>
#include <QtQml/qqmlregistration.h>
#include "telemetryclient.h"

class SpectrumWaveItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(TelemetryClient *source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(float minDb READ minDb WRITE setMinDb NOTIFY minDbChanged)
    Q_PROPERTY(float maxDb READ maxDb WRITE setMaxDb NOTIFY maxDbChanged)
    Q_PROPERTY(QColor traceColor READ traceColor WRITE setTraceColor NOTIFY traceColorChanged)

public:
    explicit SpectrumWaveItem(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    TelemetryClient *source() const { return m_source; }
    float minDb() const { return m_minDb; }
    float maxDb() const { return m_maxDb; }
    QColor traceColor() const { return m_color; }

    void setSource(TelemetryClient *s);
    void setMinDb(float v);
    void setMaxDb(float v);
    void setTraceColor(const QColor &c);

signals:
    void sourceChanged();
    void minDbChanged();
    void maxDbChanged();
    void traceColorChanged();

private:
    void onFrame(const QVector<float> &dbm);

    QPointer<TelemetryClient> m_source;
    QMetaObject::Connection m_conn;

    QVector<float> m_data;
    float m_minDb = -140.f;
    float m_maxDb = -40.f;
    QColor m_color = QColor(255, 220, 0);
};