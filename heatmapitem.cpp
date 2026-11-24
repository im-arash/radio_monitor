// heatmapitem.cpp
#include "heatmapitem.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr double kDeg2Rad = 0.01745329251994329577;
constexpr double kRad2Deg = 57.2957795130823208768;
constexpr double kMetersPerDegLat = 111320.0;

inline double wrap360(double deg)
{
    double r = std::fmod(deg, 360.0);
    return r < 0.0 ? r + 360.0 : r;
}

inline double angDiffDeg(double a, double b)
{
    double d = std::fmod(a - b + 540.0, 360.0) - 180.0;
    return d < -180.0 ? d + 360.0 : d;
}

// Jet-style ramp: blue -> cyan -> green -> yellow -> red. Returns PREMULTIPLIED ARGB.
inline QRgb heatColor(float t, float intensity)
{
    const float v = std::clamp(t * intensity, 0.f, 1.f);
    if (v <= 0.02f)
        return 0;   // fully transparent

    float r, g, b;
    if (v < 0.25f)      { r = 0.f;                  g = 4.f * v;                 b = 1.f; }
    else if (v < 0.50f) { r = 0.f;                  g = 1.f;                     b = 1.f - 4.f * (v - 0.25f); }
    else if (v < 0.75f) { r = 4.f * (v - 0.50f);    g = 1.f;                     b = 0.f; }
    else                { r = 1.f;                  g = 1.f - 4.f * (v - 0.75f); b = 0.f; }

    const int a = static_cast<int>(70.f + 165.f * v);   // stays visible at the low end
    return qPremultiply(qRgba(static_cast<int>(r * 255.f),
                              static_cast<int>(g * 255.f),
                              static_cast<int>(b * 255.f),
                              a));
}
} // namespace

HeatmapItem::HeatmapItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_hist(kBins, 0.f)
    , m_cellPos(kGrid * kGrid, -1.f)
    , m_cellDist(kGrid * kGrid, 0.f)
    , m_values(kGrid * kGrid, 0)
    , m_image(kGrid, kGrid, QImage::Format_ARGB32_Premultiplied)
{
    setAntialiasing(true);
    m_image.fill(Qt::transparent);
    rebuildLut();
    rebuildCellMap();
}

void HeatmapItem::paint(QPainter *painter)
{
    if (!painter || width() <= 0.0 || height() <= 0.0)
        return;

    const QRectF r = m_viewRect;
    if (!std::isfinite(r.x()) || !std::isfinite(r.y())
        || !std::isfinite(r.width()) || !std::isfinite(r.height())
        || r.width() <= 0.0 || r.height() <= 0.0) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->drawImage(r, m_image);
    painter->restore();
}

double HeatmapItem::metersPerDegLon() const
{
    const double lat = m_station.isValid() ? m_station.latitude() : 0.0;
    const double latRad = lat * kDeg2Rad;
    return std::max(1.0, kMetersPerDegLat * std::cos(latRad));
}

QGeoCoordinate HeatmapItem::gridTopLeft() const
{
    const double dLat = m_extent / kMetersPerDegLat;
    const double dLon = m_extent / metersPerDegLon();
    return QGeoCoordinate(m_station.latitude() + dLat, m_station.longitude() - dLon);
}

QGeoCoordinate HeatmapItem::gridBottomRight() const
{
    const double dLat = m_extent / kMetersPerDegLat;
    const double dLon = m_extent / metersPerDegLon();
    return QGeoCoordinate(m_station.latitude() - dLat, m_station.longitude() + dLon);
}

void HeatmapItem::setGridExtent(double meters)
{
    meters = std::max(50.0, meters);
    if (qFuzzyCompare(m_extent, meters)) return;
    m_extent = meters;
    emit gridChanged();
}

void HeatmapItem::reset()
{
    std::fill(m_hist.begin(), m_hist.end(), 0.f);
    m_hasBearing = false;
    m_bearing = 0.0;
    m_outliers = 0;
    publishGrid();
}

void HeatmapItem::rebuildCellMap()
{
    const double half = 0.5 * kGrid;
    for (int r = 0; r < kGrid; ++r) {
        const double dy = (half - (r + 0.5)) / half;
        for (int c = 0; c < kGrid; ++c) {
            const double dx = ((c + 0.5) - half) / half;
            const double dist2 = dx * dx + dy * dy;

            m_cellDist[r * kGrid + c] = static_cast<float>(std::sqrt(std::min(dist2, 1.0)));

            if (dist2 < 1e-6 || dist2 > 1.0) {
                m_cellPos[r * kGrid + c] = -1.f;
                continue;
            }
            const double azDeg = wrap360(std::atan2(dx, dy) * kRad2Deg);
            m_cellPos[r * kGrid + c] = static_cast<float>(azDeg * kBins / 360.0);   // fractional
        }
    }
}

void HeatmapItem::pushBearing(double rawDeg)
{
    const double b = wrap360(rawDeg);

    // Smoothed bearing (used for outlier gating / map line) - unchanged logic
    if (!m_hasBearing) {
        m_bearing = b;
        m_hasBearing = true;
        m_outliers = 0;
    } else if (std::abs(angDiffDeg(b, m_bearing)) > 35.0 && ++m_outliers < 4) {
        return;     // reject isolated outliers
    } else {
        const double alpha = (m_outliers >= 4) ? 0.6 : 0.25;
        const double delta = angDiffDeg(b, m_bearing);
        m_bearing = wrap360(m_bearing + alpha * delta);
        m_outliers = 0;
    }

    // Histogram of RAW bearings: the spread of the measurements becomes the cone width.
    constexpr float kDecay = 0.985f;        // was 0.94: remember more samples
    for (float &v : m_hist)
        v *= kDecay;

    const double centerBin = (b / 360.0) * kBins;   // raw, not the smoothed m_bearing
    constexpr double kSigmaBins = 5.0;              // 2.5 deg at 0.5 deg/bin
    constexpr int kHalf = 18;
    for (int k = -kHalf; k <= kHalf; ++k) {
        int idx = (static_cast<int>(std::llround(centerBin)) + k) % kBins;
        if (idx < 0) idx += kBins;
        const double d = (idx - centerBin);
        const double wrappedD = d - kBins * std::llround(d / kBins);
        m_hist[idx] += static_cast<float>(std::exp(-0.5 * (wrappedD * wrappedD) / (kSigmaBins * kSigmaBins)));
    }
}

void HeatmapItem::publishGrid()
{
    float peak = 0.f;
    for (float v : m_hist)
        peak = std::max(peak, v);

    std::fill(m_values.begin(), m_values.end(), quint8(0));
    if (peak > 1e-4f) {
        const float invPeak = 255.f / peak;
        const int total = kGrid * kGrid;
        for (int i = 0; i < total; ++i) {
            const float pos = m_cellPos[i];
            if (pos < 0.f) continue;

            const int i0 = static_cast<int>(pos);          // pos >= 0, so truncation == floor
            const float f = pos - static_cast<float>(i0);
            const int b0 = i0 % kBins;
            const int b1 = (i0 + 1) % kBins;
            const float h = m_hist[b0] + f * (m_hist[b1] - m_hist[b0]);   // interpolate between bins

            const float fall = 1.0f - 0.55f * m_cellDist[i];              // fades along the ray
            const float val = h * invPeak * fall;
            m_values[i] = static_cast<quint8>(std::clamp(static_cast<int>(val + 0.5f), 0, 255));
        }
    }

    rebuildImage();
    update();
}

void HeatmapItem::rebuildLut()
{
    m_lut.resize(256);
    for (int v = 0; v < 256; ++v)
        m_lut[v] = heatColor(static_cast<float>(v) / 255.f, m_intensity);
}

void HeatmapItem::rebuildImage()
{
    const QRgb *lut = m_lut.constData();
    for (int y = 0; y < kGrid; ++y) {
        auto *scan = reinterpret_cast<QRgb *>(m_image.scanLine(y));
        const quint8 *src = m_values.constData() + y * kGrid;
        for (int x = 0; x < kGrid; ++x)
            scan[x] = lut[src[x]];
    }
}

void HeatmapItem::onBearingMeasured(double bearingDeg, double /*levelDb*/)
{
    if (m_locator && m_locator->station().isValid() && m_station != m_locator->station()) {
        m_station = m_locator->station();
        emit gridChanged();
    }
    if (!m_locator || !m_locator->dfMode())
        return;

    pushBearing(bearingDeg);
    publishGrid();
}

void HeatmapItem::setLocator(TelemetryClient *l)
{
    if (m_locator == l) return;

    for (const auto &conn : std::as_const(m_conns))
        QObject::disconnect(conn);
    m_conns.clear();

    m_locator = l;
    if (m_locator) {
        if (m_locator->station().isValid())
            m_station = m_locator->station();

        m_conns << connect(m_locator, &TelemetryClient::bearingMeasured,
                           this, &HeatmapItem::onBearingMeasured);
        m_conns << connect(m_locator, &TelemetryClient::stationChanged, this, [this]() {
            if (m_locator && m_locator->station().isValid() && m_station != m_locator->station()) {
                m_station = m_locator->station();
                emit gridChanged();
            }
        });
        m_conns << connect(m_locator, &TelemetryClient::axisChanged,
                           this, &HeatmapItem::reset);
        m_conns << connect(m_locator, &TelemetryClient::dfModeChanged, this, [this]() {
            if (m_locator && !m_locator->dfMode())
                reset();
        });

        publishGrid();
    } else {
        reset();
    }
    emit locatorChanged();
}

void HeatmapItem::setViewRect(const QRectF &r)
{
    if (m_viewRect == r) return;
    m_viewRect = r;
    emit viewRectChanged();
    update();
}

void HeatmapItem::setIntensity(float v)
{
    if (qFuzzyCompare(m_intensity, v)) return;
    m_intensity = v;
    rebuildLut();
    rebuildImage();
    emit intensityChanged();
    update();
}