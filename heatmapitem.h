#pragma once
#include <QQuickRhiItem>
#include <QByteArray>
#include <QPointer>
#include <QRectF>
#include <QtQml/qqmlregistration.h>
#include "dftelemetry.h"

class HeatmapItem : public QQuickRhiItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(DfTelemetry *locator READ locator WRITE setLocator NOTIFY locatorChanged)
    Q_PROPERTY(QRectF viewRect READ viewRect WRITE setViewRect NOTIFY viewRectChanged)   // grid bounds in item pixels
    Q_PROPERTY(float intensity READ intensity WRITE setIntensity NOTIFY intensityChanged)

public:
    explicit HeatmapItem(QQuickItem *parent = nullptr);
    QQuickRhiItemRenderer *createRenderer() override;

    DfTelemetry *locator() const { return m_locator; }
    QRectF viewRect() const { return m_viewRect; }
    float intensity() const { return m_intensity; }

    void setLocator(DfTelemetry *l);
    void setViewRect(const QRectF &r);
    void setIntensity(float v);

signals:
    void locatorChanged();
    void viewRectChanged();
    void intensityChanged();

private:
    friend class HeatmapRenderer;
    void onGrid(const QByteArray &grid);

    QPointer<DfTelemetry> m_locator;
    QMetaObject::Connection m_conn;

    QByteArray m_grid;
    bool m_dirty = false;
    QRectF m_viewRect;
    float m_intensity = 0.75f;
};