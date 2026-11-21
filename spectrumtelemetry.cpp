#include "spectrumtelemetry.h"
#include <QRandomGenerator>
#include <algorithm>
#include <cmath>
#include <random>

namespace {
constexpr double kTrueBearing = 35.0;      // demo: the transmitter is 35 deg (north-east) of the station
constexpr float  kDfSquelch   = -100.f;    // no bearing below this level at the tuned frequency

// Deterministic per-channel hash so the demo "spectrum" is stable at any frequency
quint32 hashChannel(qint64 k)
{
    quint64 x = quint64(k) * 0x9E3779B97F4A7C15ULL;
    x ^= x >> 32;
    x *= 0xD6E8FEB86659FD93ULL;
    x ^= x >> 32;
    return quint32(x);
}
double wrap360(double a) { a = std::fmod(a, 360.0); return a < 0 ? a + 360.0 : a; }
}

SpectrumTelemetry::SpectrumTelemetry(QObject *parent) : QObject(parent)
{
    m_timer.setInterval(33);
    connect(&m_timer, &QTimer::timeout, this, &SpectrumTelemetry::generateDemoFrame);
}

void SpectrumTelemetry::setDemo(bool on)
{
    if (m_demo == on) return;
    m_demo = on;
    on ? m_timer.start() : m_timer.stop();
    emit demoChanged();
}

void SpectrumTelemetry::setBinCount(int n)
{
    n = std::max(2, n);
    if (m_binCount == n) return;
    m_binCount = n;
    emit binCountChanged();
}

void SpectrumTelemetry::setCenterFrequency(double hz)
{
    hz = std::max(0.0, hz);
    if (qFuzzyCompare(m_center + 1.0, hz + 1.0)) return;
    m_center = hz;
    emit centerFrequencyChanged();
    emit axisChanged();
    emit tuningChanged();
}

void SpectrumTelemetry::setSpan(double hz)
{
    hz = std::max(1.0, hz);
    if (qFuzzyCompare(m_span, hz)) return;
    m_span = hz;
    emit spanChanged();
    emit axisChanged();
    emit tuningChanged();
}

void SpectrumTelemetry::setDfMode(bool on)
{
    if (m_dfMode == on) return;
    m_dfMode = on;
    emit dfModeChanged();
    emit tuningChanged();
}

void SpectrumTelemetry::pushFrame(const QVector<float> &dbm)
{
    emit frameReady(dbm);
}

void SpectrumTelemetry::pushBearing(double bearingDeg, double levelDb)
{
    emit bearingMeasured(bearingDeg, levelDb);
}

void SpectrumTelemetry::generateDemoFrame()
{
    constexpr double channelSpacing = 12500.0;   // Hz
    constexpr double channelBw      = 9000.0;    // Hz
    constexpr double wideBw         = 25000.0;   // Hz, wideband signal width
    constexpr double wideSpacing    = 1.0e6;     // one wideband signal every 1 MHz...
    constexpr double wideOffset     = 150.0e3;   // ...at N.15 MHz (so 100.15 MHz, 101.15 MHz, ...)

    const int n = m_binCount;
    const double start = m_center - m_span / 2.0;
    m_phase += 0.05f;                            // ~1.5 per second at 30 fps

    // push-to-talk slot: a new on/off decision about every 2 seconds
    const qint64 slot = qint64(m_phase / 3.0f);

    QVector<float> d(n);
    auto *rng = QRandomGenerator::global();

    for (int i = 0; i < n; ++i) {
        const double f = start + m_span * double(i) / (n - 1);
        float v = -118.f + float(rng->bounded(6.0));            // noise floor

        // --- fixed narrowband channels (absolute frequencies) ---
        const qint64 k = qRound64(f / channelSpacing);
        const quint32 h = hashChannel(k);
        if (f > 0 && (h % 100) < 12) {
            const double off = std::abs(f - double(k) * channelSpacing);
            const double a = off / (channelBw / 2.0);
            if (a < 1.0) {
                // about 1 in 3 active channels is push-to-talk: on/off per time slot
                const bool ptt = ((h >> 16) % 3) == 0;
                const bool on  = !ptt || (hashChannel(k * 1000003 + slot) % 100) < 45;

                if (on) {
                    const float level = -55.f - float((h >> 8) % 45)
                    + 1.5f * std::sin(m_phase * 0.7f + float(k));   // gentle fading
                    const float sig = level - float(30.0 * std::pow(a, 6.0))
                                      + float(rng->bounded(3.0)) - 1.5f;
                    v = std::max(v, sig);
                }
            }
        }

        // --- wideband signal: flat top, steep edges, fixed at N.15 MHz ---
        {
            const double wc = std::round((f - wideOffset) / wideSpacing) * wideSpacing + wideOffset;
            const double off = std::abs(f - wc) / (wideBw / 2.0);
            if (off < 1.3) {
                const float sig = -78.f - float(30.0 * std::pow(off, 12.0))
                + float(rng->bounded(4.0)) - 2.f;
                v = std::max(v, sig);
            }
        }

        // --- DF demo: a signal on the tuned frequency while DF Mode is on ---
        if (m_dfMode) {
            const double t = std::abs(f - m_center) / 6000.0;
            if (t < 1.0)
                v = std::max(v, -60.f - float(30.0 * std::pow(t, 6.0)) + float(rng->bounded(3.0)));
        }

        d[i] = v;
    }
    pushFrame(d);

    // DF: the receiver measures a bearing on the signal at the center frequency
    if (m_dfMode) {
        const float level = std::max(d[(n - 1) / 2], d[n / 2]);
        if (level > kDfSquelch) {
            std::normal_distribution<double> noise(0.0, 6.0);          // urban DF scatter
            double brg = kTrueBearing + noise(*rng);
            if (rng->bounded(100) < 10)                                // multipath outlier
                brg += (rng->bounded(2) ? 1.0 : -1.0) * (15.0 + rng->bounded(20.0));
            pushBearing(wrap360(brg), level);
        }
    }
}