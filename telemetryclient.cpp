#include "telemetryclient.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <QDebug>

namespace {
constexpr double kLineLength = 600.0;
constexpr quint32 kMaxTcpFrameBytes = 1024 * 1024;
} // namespace

TelemetryClient::TelemetryClient(QObject *parent)
    : QObject(parent)
{
    connect(&m_tcpServer, &QTcpServer::newConnection,
            this, &TelemetryClient::onNewTcpConnection);
    startTcpServer();
}

TelemetryClient::~TelemetryClient()
{
    if (m_tcpClient) {
        m_tcpClient->disconnectFromHost();
    }
    m_tcpServer.close();
}

void TelemetryClient::startTcpServer()
{
    m_tcpServer.close();
    if (m_tcpServer.listen(QHostAddress::AnyIPv4, m_tcpPort)) {
        qInfo() << "TelemetryClient listening for TCP Packet stream on port" << m_tcpPort;
    } else {
        qWarning() << "Failed to listen on TCP port" << m_tcpPort << m_tcpServer.errorString();
    }
}

void TelemetryClient::setTcpPort(quint16 port)
{
    if (m_tcpPort == port)
        return;
    m_tcpPort = port;
    emit tcpPortChanged();
    if (m_tcpClient) {
        m_tcpClient->abort();
    }
    startTcpServer();
}

bool TelemetryClient::isTcpConnected() const
{
    return m_tcpClient && m_tcpClient->state() == QAbstractSocket::ConnectedState;
}

void TelemetryClient::onNewTcpConnection()
{
    QTcpSocket *next = m_tcpServer.nextPendingConnection();
    if (!next)
        return;

    if (m_tcpClient) {
        m_tcpClient->disconnect(this);
        m_tcpClient->abort();
        m_tcpClient->deleteLater();
        m_tcpClient = nullptr;
        m_tcpBuffer.clear();
    }

    m_tcpClient = next;
    m_tcpClient->setSocketOption(QAbstractSocket::LowDelayOption, 1);

    connect(m_tcpClient, &QTcpSocket::readyRead,
            this, &TelemetryClient::onTcpReadyRead);
    connect(m_tcpClient, &QTcpSocket::disconnected,
            this, &TelemetryClient::onTcpDisconnected);

    qInfo() << "TCP stream client connected from"
            << m_tcpClient->peerAddress().toString() << m_tcpClient->peerPort();
    emit tcpConnectedChanged();
}

void TelemetryClient::onTcpDisconnected()
{
    if (!m_tcpClient)
        return;

    m_tcpClient->deleteLater();
    m_tcpClient = nullptr;
    m_tcpBuffer.clear();
    emit tcpConnectedChanged();
}

void TelemetryClient::onTcpReadyRead()
{
    if (!m_tcpClient)
        return;

    m_tcpBuffer.append(m_tcpClient->readAll());
    processTcpBuffer();
}

void TelemetryClient::processTcpBuffer()
{
    constexpr int headerBytes = static_cast<int>(sizeof(quint32));
    constexpr quint32 headerOnlyBytes = static_cast<quint32>(offsetof(faraz::Packet, bins));

    while (m_tcpBuffer.size() >= headerBytes) {
        const auto *raw = reinterpret_cast<const uchar *>(m_tcpBuffer.constData());
        const quint32 frameSize = (static_cast<quint32>(raw[0]) << 24)
                                  | (static_cast<quint32>(raw[1]) << 16)
                                  | (static_cast<quint32>(raw[2]) << 8)
                                  | (static_cast<quint32>(raw[3]));

        if (frameSize == 0 || frameSize > kMaxTcpFrameBytes) {
            qWarning() << "Invalid TCP frame size:" << frameSize << "- resetting buffer";
            m_tcpBuffer.clear();
            return;
        }

        const int totalNeeded = headerBytes + static_cast<int>(frameSize);
        if (m_tcpBuffer.size() < totalNeeded) {
            return;
        }

        if (frameSize >= headerOnlyBytes && frameSize <= sizeof(faraz::Packet)) {
            faraz::Packet pkt{};
            std::memcpy(&pkt, raw + headerBytes, static_cast<size_t>(frameSize));

            const quint32 maxPossibleBins =
                (frameSize - headerOnlyBytes) / static_cast<quint32>(sizeof(float));
            if (pkt.binCount > maxPossibleBins) {
                pkt.binCount = maxPossibleBins;
            }

            if (pkt.magic == faraz::kPacketMagic && pkt.version == faraz::kPacketVersion) {
                handlePacket(pkt);
            } else {
                qWarning() << "Invalid Packet header magic/version";
            }
        } else {
            qWarning() << "Unexpected Packet payload size:" << frameSize;
        }

        m_tcpBuffer.remove(0, totalNeeded);
    }
}

void TelemetryClient::handlePacket(const faraz::Packet &pkt)
{
    emit packetReceived(pkt);

    if ((pkt.flags & faraz::PacketFlagHasCoord) != 0
        && std::isfinite(pkt.latitude) && std::isfinite(pkt.longitude)) {
        const QGeoCoordinate coord(pkt.latitude, pkt.longitude);
        if (coord.isValid() && m_station != coord) {
            m_station = coord;
            emit stationChanged();
        }
    }

    bool axisUpdated = false;
    if (pkt.centerFrequencyHz > 0.0 && std::isfinite(pkt.centerFrequencyHz)
        && !qFuzzyCompare(m_center + 1.0, pkt.centerFrequencyHz + 1.0)) {
        m_center = pkt.centerFrequencyHz;
        emit centerFrequencyChanged();
        axisUpdated = true;
    }

    if (pkt.spanHz > 0.0 && std::isfinite(pkt.spanHz)
        && !qFuzzyCompare(m_span, pkt.spanHz)) {
        m_span = pkt.spanHz;
        emit spanChanged();
        axisUpdated = true;
    }

    const bool pktDfMode = (pkt.flags & faraz::PacketFlagDfMode) != 0;
    if (m_dfMode != pktDfMode) {
        m_dfMode = pktDfMode;
        if (!m_dfMode) {
            resetDf();
        }
        emit dfModeChanged();
        emit activeChanged();
    }

    if ((pkt.flags & faraz::PacketFlagResetAxis) != 0 || axisUpdated) {
        emit axisChanged();
    }

    if ((pkt.flags & faraz::PacketFlagResetDf) != 0) {
        resetDf();
    }

    if ((pkt.flags & faraz::PacketFlagHasSpec) != 0 && pkt.binCount > 0) {
        const int count = static_cast<int>(
            std::min<quint32>(pkt.binCount, static_cast<quint32>(faraz::kMaxPacketBins)));
        QVector<float> dbm(count);
        for (int i = 0; i < count; ++i) {
            const float v = pkt.bins[i];
            dbm[i] = std::isfinite(v) ? v : -120.f;
        }
        emit frameReady(dbm);
    }

    if ((pkt.flags & faraz::PacketFlagHasDf) != 0
        && std::isfinite(pkt.bearingDeg) && std::isfinite(pkt.levelDbm)) {
        if (m_dfMode) {
            m_bearing = pkt.bearingDeg;
            m_hasBearing = true;
            emit bearingChanged();
        }
        emit bearingMeasured(pkt.bearingDeg, pkt.levelDbm);
    }
}

QGeoCoordinate TelemetryClient::lineEnd() const
{
    if (!m_station.isValid())
        return QGeoCoordinate();
    return m_station.atDistanceAndAzimuth(kLineLength, m_bearing);
}

void TelemetryClient::setCenterFrequency(double hz)
{
    hz = std::max(0.0, hz);
    if (qFuzzyCompare(m_center + 1.0, hz + 1.0))
        return;
    m_center = hz;
    emit centerFrequencyChanged();
    emit axisChanged();
    emit tuningChanged();
}

void TelemetryClient::setSpan(double hz)
{
    hz = std::max(1.0, hz);
    if (qFuzzyCompare(m_span, hz))
        return;
    m_span = hz;
    emit spanChanged();
    emit axisChanged();
    emit tuningChanged();
}

void TelemetryClient::setDfMode(bool on)
{
    if (m_dfMode == on)
        return;
    m_dfMode = on;
    if (!m_dfMode)
        resetDf();
    emit dfModeChanged();
    emit activeChanged();
    emit tuningChanged();
}

void TelemetryClient::resetDf()
{
    if (!m_hasBearing && qFuzzyIsNull(m_bearing))
        return;
    m_hasBearing = false;
    m_bearing = 0.0;
    emit bearingChanged();
}