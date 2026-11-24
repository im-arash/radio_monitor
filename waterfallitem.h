// waterfallitem.h
#pragma once

#include <QQuickPaintedItem>
#include <QImage>
#include <QPointer>
#include <QVector>
#include <QtQml/qqmlregistration.h>
#include "telemetryclient.h"

class WaterfallItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(TelemetryClient *source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(float minDb READ minDb WRITE setMinDb NOTIFY minDbChanged)
    Q_PROPERTY(float maxDb READ maxDb WRITE setMaxDb NOTIFY maxDbChanged)

public:
    static constexpr int kTexW = 1024;     // columns (frequency)  - was 2048, source only has ~256 bins
    static constexpr int kTexH = 128;      // rows (history): ~3 px per row, fills in ~13 s at 10 Hz
    static constexpr float kStoreMin = -160.f;
    static constexpr float kStoreMax = 0.f;

    explicit WaterfallItem(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    TelemetryClient *source() const { return m_source; }
    float minDb() const { return m_minDb; }
    float maxDb() const { return m_maxDb; }

    void setSource(TelemetryClient *s);
    void setMinDb(float v);
    void setMaxDb(float v);

signals:
    void sourceChanged();
    void minDbChanged();
    void maxDbChanged();

private:
    void onFrame(const QVector<float> &dbm);
    void onTuningChanged();
    void rebuildColormap();
    void clearHistory();
    void recolorRow(int row);
    void recolorAll();

    QPointer<TelemetryClient> m_source;
    QMetaObject::Connection m_frameConn, m_tuneConn;

    QImage m_history;               // kTexW x kTexH Grayscale8 ring buffer (raw dB, 8-bit)
    QImage m_colored;               // kTexW x kTexH RGB32, same ring layout, already colorized
    QVector<QRgb> m_colormap;       // 256-entry lookup table for [m_minDb, m_maxDb]
    int m_head = 0;                 // next row index to write (decrements; newest = head + 1)
    int m_filledRows = 0;           // number of valid rows currently in ring buffer
    float m_minDb = -140.f;
    float m_maxDb = -40.f;
};