// stationstate.h
#pragma once

#include "packet.h"

#include <QObject>
#include <QString>
#include <QVector>
#include <QGeoCoordinate>
#include <QtQml/qqmlregistration.h>

class StationState : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(int tcpPort READ tcpPort WRITE setTcpPort NOTIFY tcpPortChanged)
    Q_PROPERTY(double centerFrequency READ centerFrequency WRITE setCenterFrequency NOTIFY centerFrequencyChanged)
    Q_PROPERTY(double span READ span WRITE setSpan NOTIFY spanChanged)
    Q_PROPERTY(bool dfMode READ dfMode WRITE setDfMode NOTIFY dfModeChanged)

    Q_PROPERTY(bool active READ active NOTIFY activeChanged)
    Q_PROPERTY(QGeoCoordinate station READ station NOTIFY stationChanged)
    Q_PROPERTY(bool hasBearing READ hasBearing NOTIFY bearingChanged)
    Q_PROPERTY(double bearing READ bearing NOTIFY bearingChanged)
    Q_PROPERTY(QGeoCoordinate lineEnd READ lineEnd NOTIFY bearingChanged)

public:
    explicit StationState(QObject *parent = nullptr);

    QString name() const { return m_name; }
    int tcpPort() const { return m_tcpPort; }
    double centerFrequency() const { return m_center; }
    double span() const { return m_span; }
    bool dfMode() const { return m_dfMode; }

    bool active() const { return m_dfMode; }
    QGeoCoordinate station() const { return m_station; }
    bool hasBearing() const { return m_hasBearing; }
    double bearing() const { return m_bearing; }
    QGeoCoordinate lineEnd() const;

    void setName(const QString &name);
    void setTcpPort(int port);
    void setCenterFrequency(double hz);
    void setSpan(double hz);
    void setDfMode(bool on);

    Q_INVOKABLE void resetDf();

public slots:
    void applyPacket(const faraz::Packet &pkt);

signals:
    void nameChanged();
    void tcpPortChanged();
    void frameReady(const QVector<float> &dbm);
    void bearingMeasured(double bearingDeg, double levelDb);
    void tuningChanged();
    void axisChanged();

    void centerFrequencyChanged();
    void spanChanged();
    void dfModeChanged();

    void activeChanged();
    void stationChanged();
    void bearingChanged();

private:
    QString m_name;
    int m_tcpPort = 0;
    double m_center = 100e6;
    double m_span = 500e3;
    bool m_dfMode = false;

    QGeoCoordinate m_station;
    bool m_hasBearing = false;
    double m_bearing = 0.0;
};