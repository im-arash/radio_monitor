#include "spectrumwaveitem.h"
#include <QPainter>
#include <QPen>
#include <QPolygonF>
#include <QtMath>
#include <algorithm>

SpectrumWaveItem::SpectrumWaveItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setAntialiasing(true);
}

void SpectrumWaveItem::paint(QPainter *painter)
{
    const int count = static_cast<int>(m_data.size());
    if (count < 2)
        return;

    const QRectF rect = boundingRect();
    if (rect.width() <= 0.0 || rect.height() <= 0.0)
        return;

    const float spanDb = m_maxDb - m_minDb;
    if (qFuzzyIsNull(spanDb))
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QPen pen(m_color, 1.0);
    pen.setCosmetic(true);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);

    QPolygonF points;
    points.reserve(count);

    const qreal x0 = rect.left();
    const qreal yBottom = rect.bottom();
    const qreal width = rect.width();
    const qreal height = rect.height();
    const qreal invSpan = 1.0 / static_cast<qreal>(spanDb);
    const qreal stepX = width / static_cast<qreal>(count - 1);

    for (int i = 0; i < count; ++i) {
        const qreal x = x0 + static_cast<qreal>(i) * stepX;
        const qreal normY = std::clamp((static_cast<qreal>(m_data[i]) - static_cast<qreal>(m_minDb)) * invSpan,
                                       0.0, 1.0);
        const qreal y = yBottom - normY * height;
        points.append(QPointF(x, y));
    }

    painter->drawPolyline(points);
    painter->restore();
}

void SpectrumWaveItem::setSource(StationState *s)
{
    if (m_source == s)
        return;

    QObject::disconnect(m_conn);
    m_source = s;

    m_data.clear();

    if (m_source) {
        m_conn = connect(m_source, &StationState::frameReady,
                         this, &SpectrumWaveItem::onFrame);
    }
    emit sourceChanged();
    update();
}

void SpectrumWaveItem::onFrame(const QVector<float> &dbm)
{
    m_data = dbm;
    update();
}

void SpectrumWaveItem::setMinDb(float v)
{
    if (qFuzzyCompare(m_minDb, v))
        return;
    m_minDb = v;
    emit minDbChanged();
    update();
}

void SpectrumWaveItem::setMaxDb(float v)
{
    if (qFuzzyCompare(m_maxDb, v))
        return;
    m_maxDb = v;
    emit maxDbChanged();
    update();
}

void SpectrumWaveItem::setTraceColor(const QColor &c)
{
    if (m_color == c)
        return;
    m_color = c;
    emit traceColorChanged();
    update();
}