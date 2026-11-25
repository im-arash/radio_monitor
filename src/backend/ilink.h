//ilink.h
#pragma once
#include "packet.h"
#include <QObject>

class ILink : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    virtual bool listen(quint16 port) = 0;
    virtual void close() = 0;
    virtual bool sendTuning(double centerFrequencyHz, double spanHz, bool dfMode) = 0;

signals:
    void packetReceived(const faraz::Packet &packet);
    void clientConnected();
    void clientDisconnected();
};