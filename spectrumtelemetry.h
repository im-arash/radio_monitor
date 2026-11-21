#pragma once
#include <QObject>
#include <QTimer>
#include <QVector>
#include <QtQml/qqmlregistration.h>

class SpectrumTelemetry : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool demo READ demo WRITE setDemo NOTIFY demoChanged)
    Q_PROPERTY(int binCount READ binCount WRITE setBinCount NOTIFY binCountChanged)
    Q_PROPERTY(double centerFrequency READ centerFrequency WRITE setCenterFrequency NOTIFY centerFrequencyChanged) // Hz
    Q_PROPERTY(double span READ span WRITE setSpan NOTIFY spanChanged)                                             // Hz
    Q_PROPERTY(bool dfMode READ dfMode WRITE setDfMode NOTIFY dfModeChanged)

public:
    explicit SpectrumTelemetry(QObject *parent = nullptr);

    bool demo() const { return m_demo; }
    int binCount() const { return m_binCount; }
    double centerFrequency() const { return m_center; }
    double span() const { return m_span; }
    bool dfMode() const { return m_dfMode; }

    void setDemo(bool on);
    void setBinCount(int n);
    void setCenterFrequency(double hz);
    void setSpan(double hz);
    void setDfMode(bool on);

public slots:
    // Real hardware: call these from your receiver code
    void pushFrame(const QVector<float> &dbm);                      // one value per bin, dBm
    void pushBearing(double bearingDeg, double levelDb);            // DF result for the tuned frequency

signals:
    void frameReady(const QVector<float> &dbm);
    void bearingMeasured(double bearingDeg, double levelDb);        // degrees clockwise from north
    void tuningChanged();                                           // center/span/dfMode changed: retune hardware
    void axisChanged();                                             // center or span changed

    void demoChanged();
    void binCountChanged();
    void centerFrequencyChanged();
    void spanChanged();
    void dfModeChanged();

private:
    void generateDemoFrame();

    QTimer m_timer;
    bool m_demo = false;
    int m_binCount = 1024;
    double m_center = 100e6;
    double m_span = 500e3;
    bool m_dfMode = false;
    float m_phase = 0.f;
};