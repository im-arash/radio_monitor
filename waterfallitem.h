// waterfallitem.h
#pragma once

#include <QQuickPaintedItem>
#include <QImage>
#include <QPointer>
#include <QVector>
#include <QtQml/qqmlregistration.h>
#include "stationstate.h"

class WaterfallItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(StationState *source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(float minDb READ minDb WRITE setMinDb NOTIFY minDbChanged)
    Q_PROPERTY(float maxDb READ maxDb WRITE setMaxDb NOTIFY maxDbChanged)

public:
    static constexpr int kTexW = 1024;
    static constexpr int kTexH = 128;
    static constexpr float kStoreMin = -160.f;
    static constexpr float kStoreMax = 0.f;

    explicit WaterfallItem(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    StationState *source() const { return m_source; }
    float minDb() const { return m_minDb; }
    float maxDb() const { return m_maxDb; }

    void setSource(StationState *s);
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

    QPointer<StationState> m_source;
    QMetaObject::Connection m_frameConn, m_tuneConn;

    QImage m_history;
    QImage m_colored;
    QVector<QRgb> m_colormap;
    int m_head = 0;
    int m_filledRows = 0;
    float m_minDb = -140.f;
    float m_maxDb = -40.f;
};