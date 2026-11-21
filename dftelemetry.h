#pragma once
#include <QObject>
#include <QByteArray>
#include <QGeoCoordinate>
#include <QVector>
#include <QtQml/qqmlregistration.h>

// One stationary DF station: smoothed bearing line + azimuth-histogram heatmap.
class DfTelemetry : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QGeoCoordinate station READ station WRITE setStation NOTIFY stationChanged)
    Q_PROPERTY(bool hasBearing READ hasBearing NOTIFY bearingChanged)
    Q_PROPERTY(double bearing READ bearing NOTIFY bearingChanged)                  // smoothed, degrees
    Q_PROPERTY(QGeoCoordinate lineEnd READ lineEnd NOTIFY bearingChanged)

    // heat grid: square, centered on the station
    Q_PROPERTY(double gridExtent READ gridExtent WRITE setGridExtent NOTIFY gridChanged)   // meters
    Q_PROPERTY(QGeoCoordinate gridTopLeft READ gridTopLeft NOTIFY gridChanged)
    Q_PROPERTY(QGeoCoordinate gridBottomRight READ gridBottomRight NOTIFY gridChanged)

public:
    static constexpr int kGrid = 128;
    static constexpr int kBins = 360;              // azimuth histogram, 1 degree per bin

    explicit DfTelemetry(QObject *parent = nullptr);

    bool active() const { return m_active; }
    QGeoCoordinate station() const { return m_station; }
    bool hasBearing() const { return m_has; }
    double bearing() const { return m_bearing; }
    QGeoCoordinate lineEnd() const;
    double gridExtent() const { return m_extent; }
    QGeoCoordinate gridTopLeft() const;
    QGeoCoordinate gridBottomRight() const;
    const QByteArray &grid() const { return m_grid; }

    void setActive(bool on);
    void setStation(const QGeoCoordinate &c);
    void setGridExtent(double meters);

    Q_INVOKABLE void pushBearing(double bearingDeg);   // degrees clockwise from north
    Q_INVOKABLE void reset();

signals:
    void activeChanged();
    void stationChanged();
    void bearingChanged();
    void gridChanged();
    void gridReady(const QByteArray &grid);            // kGrid*kGrid bytes, row 0 = north

private:
    void publishGrid();
    double metersPerDegLon() const;

    bool m_active = false;
    QGeoCoordinate m_station{35.700956, 51.340737};
    double m_extent = 1400.0;

    bool m_has = false;
    double m_bearing = 0.0;
    int m_outliers = 0;

    QVector<float> m_hist;          // kBins
    QVector<qint16> m_cellBin;      // per cell: azimuth bin, or -1 outside the circle
    QByteArray m_grid;
};