// heatmapitem.h
#pragma once
#include <QQuickPaintedItem>
#include <QPainter>
#include <QImage>
#include <QPointer>
#include <QRectF>
#include <QVector>
#include <QList>
#include <QGeoCoordinate>
#include <QtQml/qqmlregistration.h>
#include "stationstate.h"

class HeatmapItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(StationState *locator READ locator WRITE setLocator NOTIFY locatorChanged)
    Q_PROPERTY(QRectF viewRect READ viewRect WRITE setViewRect NOTIFY viewRectChanged)
    Q_PROPERTY(float intensity READ intensity WRITE setIntensity NOTIFY intensityChanged)
    Q_PROPERTY(double gridExtent READ gridExtent WRITE setGridExtent NOTIFY gridChanged)
    Q_PROPERTY(QGeoCoordinate gridTopLeft READ gridTopLeft NOTIFY gridChanged)
    Q_PROPERTY(QGeoCoordinate gridBottomRight READ gridBottomRight NOTIFY gridChanged)

public:
    static constexpr int kGrid = 256;
    static constexpr int kBins = 720;

    explicit HeatmapItem(QQuickItem *parent = nullptr);
    void paint(QPainter *painter) override;

    StationState *locator() const { return m_locator; }
    QRectF viewRect() const { return m_viewRect; }
    float intensity() const { return m_intensity; }
    double gridExtent() const { return m_extent; }
    QGeoCoordinate gridTopLeft() const;
    QGeoCoordinate gridBottomRight() const;

    void setLocator(StationState *l);
    void setViewRect(const QRectF &r);
    void setIntensity(float v);
    void setGridExtent(double meters);

    Q_INVOKABLE void reset();

signals:
    void locatorChanged();
    void viewRectChanged();
    void intensityChanged();
    void gridChanged();

private:
    void onBearingMeasured(double bearingDeg, double levelDb);
    void pushBearing(double rawDeg);
    void rebuildCellMap();
    void publishGrid();
    void rebuildImage();
    void rebuildLut();
    double metersPerDegLon() const;

    QPointer<StationState> m_locator;
    QList<QMetaObject::Connection> m_conns;

    QGeoCoordinate m_station;
    double m_extent = 1400.0;
    bool m_hasBearing = false;
    double m_bearing = 0.0;
    int m_outliers = 0;

    QVector<float> m_hist;
    QVector<float> m_cellPos;
    QVector<QRgb> m_lut;
    QVector<float> m_cellDist;
    QVector<quint8> m_values;
    QImage m_image;

    QRectF m_viewRect;
    float m_intensity = 1.0f;
};