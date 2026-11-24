#pragma once

#include "packet.h"

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QVector>
#include <QGeoCoordinate>
#include <QtQml/qqmlregistration.h>

class TelemetryClient : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(double centerFrequency READ centerFrequency WRITE setCenterFrequency NOTIFY centerFrequencyChanged)
    Q_PROPERTY(double span READ span WRITE setSpan NOTIFY spanChanged)
    Q_PROPERTY(bool dfMode READ dfMode WRITE setDfMode NOTIFY dfModeChanged)

    Q_PROPERTY(bool active READ active NOTIFY activeChanged)
    Q_PROPERTY(QGeoCoordinate station READ station NOTIFY stationChanged)
    Q_PROPERTY(bool hasBearing READ hasBearing NOTIFY bearingChanged)
    Q_PROPERTY(double bearing READ bearing NOTIFY bearingChanged)
    Q_PROPERTY(QGeoCoordinate lineEnd READ lineEnd NOTIFY bearingChanged)

    Q_PROPERTY(quint16 tcpPort READ tcpPort WRITE setTcpPort NOTIFY tcpPortChanged)
    Q_PROPERTY(bool tcpConnected READ isTcpConnected NOTIFY tcpConnectedChanged)

public:
    explicit TelemetryClient(QObject *parent = nullptr);
    ~TelemetryClient() override;

    double centerFrequency() const { return m_center; }
    double span() const { return m_span; }
    bool dfMode() const { return m_dfMode; }

    bool active() const { return m_dfMode; }
    QGeoCoordinate station() const { return m_station; }
    bool hasBearing() const { return m_hasBearing; }
    double bearing() const { return m_bearing; }
    QGeoCoordinate lineEnd() const;

    quint16 tcpPort() const { return m_tcpPort; }
    bool isTcpConnected() const;

    void setCenterFrequency(double hz);
    void setSpan(double hz);
    void setDfMode(bool on);
    void setTcpPort(quint16 port);

    Q_INVOKABLE void resetDf();

signals:
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

    void tcpPortChanged();
    void tcpConnectedChanged();
    void packetReceived(const faraz::Packet &pkt);

private slots:
    void onNewTcpConnection();
    void onTcpReadyRead();
    void onTcpDisconnected();

private:
    void startTcpServer();
    void processTcpBuffer();
    void handlePacket(const faraz::Packet &pkt);

    QTcpServer m_tcpServer;
    QTcpSocket *m_tcpClient = nullptr;
    QByteArray m_tcpBuffer;
    quint16 m_tcpPort = 9000;

    double m_center = 100e6;
    double m_span = 500e3;
    bool m_dfMode = false;

    QGeoCoordinate m_station;
    bool m_hasBearing = false;
    double m_bearing = 0.0;
};