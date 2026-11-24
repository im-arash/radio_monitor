// stationstate.cpp
#include "stationstate.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr double kLineLength = 600.0;
} // namespace

StationState::StationState(QObject *parent)
    : QObject(parent)
{
}

void StationState::setName(const QString &name)
{
    if (m_name == name)
        return;
    m_name = name;
    emit nameChanged();
}

void StationState::setTcpPort(int port)
{
    if (port < 0 || port > 65535 || m_tcpPort == port)
        return;
    m_tcpPort = port;
    emit tcpPortChanged();
}

void StationState::applyPacket(const faraz::Packet &pkt)
{
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

QGeoCoordinate StationState::lineEnd() const
{
    if (!m_station.isValid())
        return QGeoCoordinate();
    return m_station.atDistanceAndAzimuth(kLineLength, m_bearing);
}

void StationState::setCenterFrequency(double hz)
{
    hz = std::max(0.0, hz);
    if (qFuzzyCompare(m_center + 1.0, hz + 1.0))
        return;
    m_center = hz;
    emit centerFrequencyChanged();
    emit axisChanged();
    emit tuningChanged();
}

void StationState::setSpan(double hz)
{
    hz = std::max(1.0, hz);
    if (qFuzzyCompare(m_span, hz))
        return;
    m_span = hz;
    emit spanChanged();
    emit axisChanged();
    emit tuningChanged();
}

void StationState::setDfMode(bool on)
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

void StationState::resetDf()
{
    if (!m_hasBearing && qFuzzyIsNull(m_bearing))
        return;
    m_hasBearing = false;
    m_bearing = 0.0;
    emit bearingChanged();
}