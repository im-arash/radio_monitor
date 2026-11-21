#include "dftelemetry.h"
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace {
constexpr double kMetersPerDegLat = 111320.0;
constexpr double kLineLength = 600.0;     // meters
constexpr double kSigmaDeg = 2.5;         // width each measurement paints into the histogram
constexpr float  kDecay = 0.999f;         // history fades (half-life ~23 s at 30 Hz)
double wrap360(double a) { a = std::fmod(a, 360.0); return a < 0 ? a + 360.0 : a; }
}

DfTelemetry::DfTelemetry(QObject *parent) : QObject(parent)
{
    m_hist.fill(0.f, kBins);
    m_grid = QByteArray(kGrid * kGrid, '\0');
    m_cellBin.fill(-1, kGrid * kGrid);

    // Station is at the grid center, so each cell's azimuth never changes: precompute it
    const double cell = 2.0 / kGrid;                        // unit square
    for (int r = 0; r < kGrid; ++r) {
        for (int c = 0; c < kGrid; ++c) {
            const double x = -1.0 + (c + 0.5) * cell;       // east
            const double y =  1.0 - (r + 0.5) * cell;       // north
            if (x * x + y * y > 1.0) continue;              // keep a circle of radius extent/2
            double deg = qRadiansToDegrees(std::atan2(x, y));
            if (deg < 0) deg += 360.0;
            m_cellBin[r * kGrid + c] = qint16(int(deg) % kBins);
        }
    }
}

double DfTelemetry::metersPerDegLon() const
{
    return kMetersPerDegLat * std::cos(qDegreesToRadians(m_station.latitude()));
}

QGeoCoordinate DfTelemetry::lineEnd() const
{
    return m_station.atDistanceAndAzimuth(kLineLength, m_bearing);
}

QGeoCoordinate DfTelemetry::gridTopLeft() const
{
    const double half = m_extent / 2.0;
    return QGeoCoordinate(m_station.latitude() + half / kMetersPerDegLat,
                          m_station.longitude() - half / metersPerDegLon());
}

QGeoCoordinate DfTelemetry::gridBottomRight() const
{
    const double half = m_extent / 2.0;
    return QGeoCoordinate(m_station.latitude() - half / kMetersPerDegLat,
                          m_station.longitude() + half / metersPerDegLon());
}

void DfTelemetry::setActive(bool on)
{
    if (m_active == on) return;
    m_active = on;
    if (!on) reset();
    emit activeChanged();
}

void DfTelemetry::setStation(const QGeoCoordinate &c)
{
    if (m_station == c) return;
    m_station = c;
    reset();
    emit stationChanged();
    emit gridChanged();
}

void DfTelemetry::setGridExtent(double meters)
{
    meters = std::max(10.0, meters);
    if (qFuzzyCompare(m_extent, meters)) return;
    m_extent = meters;
    reset();
    emit gridChanged();
}

void DfTelemetry::reset()
{
    m_hist.fill(0.f);
    m_has = false;
    m_outliers = 0;
    publishGrid();
    emit bearingChanged();
}

void DfTelemetry::pushBearing(double raw)
{
    if (!m_active) return;
    raw = wrap360(raw);

    // 1) blue line: smoothed, ignores short outliers
    if (!m_has) {
        m_bearing = raw;
        m_has = true;
        m_outliers = 0;
    } else {
        const double d = std::remainder(raw - m_bearing, 360.0);
        if (std::abs(d) < 15.0) {
            m_bearing = wrap360(m_bearing + 0.15 * d);
            m_outliers = 0;
        } else if (++m_outliers > 5) {                 // persistent change: accept it
            m_bearing = raw;
            m_outliers = 0;
        }
    }

    // 2) heat: raw bearing goes into the azimuth histogram (how often each direction was measured)
    for (float &h : m_hist) h *= kDecay;
    const int base = int(std::floor(raw));
    for (int k = -8; k <= 8; ++k) {
        const int bin = ((base + k) % kBins + kBins) % kBins;
        const double d = std::remainder((bin + 0.5) - raw, 360.0);
        m_hist[bin] += float(std::exp(-0.5 * d * d / (kSigmaDeg * kSigmaDeg)));
    }

    publishGrid();
    emit bearingChanged();
}

void DfTelemetry::publishGrid()
{
    float peak = 0.f;
    for (float h : std::as_const(m_hist)) peak = std::max(peak, h);
    const float k = peak > 1e-6f ? 255.f / peak : 0.f;

    auto *out = reinterpret_cast<quint8 *>(m_grid.data());
    for (int i = 0; i < kGrid * kGrid; ++i) {
        const int b = m_cellBin[i];
        out[i] = b < 0 ? 0 : quint8(std::clamp(m_hist[b] * k + 0.5f, 0.f, 255.f));
    }
    emit gridReady(m_grid);
}