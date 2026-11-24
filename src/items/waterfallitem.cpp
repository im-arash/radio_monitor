// waterfallitem.cpp
#include "waterfallitem.h"
#include <QPainter>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace {
// Smooth 5-stop thermal/turbo-like palette (black -> blue -> cyan -> yellow -> white)
QRgb samplePalette(float t)
{
    t = std::clamp(t, 0.f, 1.f);

    struct Stop {
        float pos;
        int r, g, b;
    };
    static constexpr Stop stops[] = {
        { 0.00f,   0,   0,  10 },
        { 0.25f,  15,  40, 150 },
        { 0.55f,   0, 200, 210 },
        { 0.82f, 255, 225,   0 },
        { 1.00f, 255, 255, 255 }
    };
    constexpr int numStops = static_cast<int>(sizeof(stops) / sizeof(stops[0]));

    for (int i = 0; i < numStops - 1; ++i) {
        if (t <= stops[i + 1].pos) {
            const float localT = (t - stops[i].pos) / (stops[i + 1].pos - stops[i].pos);
            const int r = static_cast<int>(std::lround(stops[i].r + localT * (stops[i + 1].r - stops[i].r)));
            const int g = static_cast<int>(std::lround(stops[i].g + localT * (stops[i + 1].g - stops[i].g)));
            const int b = static_cast<int>(std::lround(stops[i].b + localT * (stops[i + 1].b - stops[i].b)));
            return qRgb(std::clamp(r, 0, 255),
                        std::clamp(g, 0, 255),
                        std::clamp(b, 0, 255));
        }
    }
    return qRgb(255, 255, 255);
}
} // namespace

WaterfallItem::WaterfallItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_history(kTexW, kTexH, QImage::Format_Grayscale8)
    , m_colored(kTexW, kTexH, QImage::Format_RGB32)
{
    m_history.fill(0);
    m_colored.fill(Qt::black);
    m_head = kTexH - 1;
    rebuildColormap();
}

void WaterfallItem::rebuildColormap()
{
    m_colormap.resize(256);

    const float spanDb = m_maxDb - m_minDb;
    if (qFuzzyIsNull(spanDb)) {
        m_colormap.fill(qRgb(0, 0, 0));
        recolorAll();
        return;
    }

    constexpr float storeSpan = kStoreMax - kStoreMin;
    for (int v = 0; v < 256; ++v) {
        // Convert stored 8-bit value back to dBm, then normalize into [m_minDb, m_maxDb]
        const float db = kStoreMin + (static_cast<float>(v) / 255.f) * storeSpan;
        const float norm = (db - m_minDb) / spanDb;
        m_colormap[v] = samplePalette(norm);
    }
    recolorAll();   // palette changed: recolor whole history once (not per frame)
}

void WaterfallItem::recolorRow(int row)
{
    if (m_colormap.size() != 256)
        return;
    const uchar *src = m_history.constScanLine(row);
    QRgb *dst = reinterpret_cast<QRgb *>(m_colored.scanLine(row));
    const QRgb *lut = m_colormap.constData();
    for (int x = 0; x < kTexW; ++x)
        dst[x] = lut[src[x]];
}

void WaterfallItem::recolorAll()
{
    for (int r = 0; r < kTexH; ++r)
        recolorRow(r);
}

void WaterfallItem::clearHistory()
{
    m_history.fill(0);
    m_colored.fill(Qt::black);
    m_head = kTexH - 1;
    m_filledRows = 0;
}

void WaterfallItem::paint(QPainter *painter)
{
    const QRectF targetRect = boundingRect();
    if (targetRect.width() <= 0.0 || targetRect.height() <= 0.0)
        return;

    painter->save();
    painter->fillRect(targetRect, Qt::black);

    if (m_filledRows > 0) {
        const int start = (m_head + 1) % kTexH;
        const int a = std::min(m_filledRows, kTexH - start);   // rows before the wrap
        const int b = m_filledRows - a;                        // rows after the wrap
        const qreal rowHeight = targetRect.height() / static_cast<qreal>(kTexH);

        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

        painter->drawImage(QRectF(targetRect.left(), targetRect.top(),
                                  targetRect.width(), rowHeight * a),
                           m_colored,
                           QRectF(0, start, kTexW, a));

        if (b > 0) {
            painter->drawImage(QRectF(targetRect.left(), targetRect.top() + rowHeight * a,
                                      targetRect.width(), rowHeight * b),
                               m_colored,
                               QRectF(0, 0, kTexW, b));
        }
    }

    painter->restore();
}

void WaterfallItem::setSource(StationState *s)
{
    if (m_source == s)
        return;

    QObject::disconnect(m_frameConn);
    QObject::disconnect(m_tuneConn);
    m_source = s;

    clearHistory();

    if (m_source) {
        m_frameConn = connect(m_source, &StationState::frameReady,
                              this, &WaterfallItem::onFrame);
        m_tuneConn  = connect(m_source, &StationState::axisChanged,
                             this, &WaterfallItem::onTuningChanged);
    }
    emit sourceChanged();
    update();
}

void WaterfallItem::onTuningChanged()
{
    clearHistory();
    update();
}

void WaterfallItem::onFrame(const QVector<float> &dbm)
{
    const int n = static_cast<int>(dbm.size());
    if (n < 2)
        return;

    uchar *dstRow = m_history.scanLine(m_head);
    constexpr float invRange = 255.f / (kStoreMax - kStoreMin);

    for (int x = 0; x < kTexW; ++x) {
        const int i0 = static_cast<int>(static_cast<qint64>(x) * n / kTexW);
        const int i1 = std::min(n, std::max(i0 + 1, static_cast<int>(static_cast<qint64>(x + 1) * n / kTexW)));
        float m = dbm[i0];
        for (int i = i0 + 1; i < i1; ++i)
            m = std::max(m, dbm[i]);

        const float t = (m - kStoreMin) * invRange;
        dstRow[x] = static_cast<uchar>(std::clamp(t + 0.5f, 0.f, 255.f));
    }

    recolorRow(m_head);                              // colorize ONLY the new row

    m_head = (m_head - 1 + kTexH) % kTexH;           // decrement: newest is always head+1
    m_filledRows = std::min(m_filledRows + 1, kTexH);
    update();
}

void WaterfallItem::setMinDb(float v)
{
    if (qFuzzyCompare(m_minDb, v))
        return;
    m_minDb = v;
    rebuildColormap();
    emit minDbChanged();
    update();
}

void WaterfallItem::setMaxDb(float v)
{
    if (qFuzzyCompare(m_maxDb, v))
        return;
    m_maxDb = v;
    rebuildColormap();
    emit maxDbChanged();
    update();
}