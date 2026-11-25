// tcplink.h
#pragma once

#include "packet.h"
#include "ilink.h"

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QHostAddress>
#include <QtQml/qqmlregistration.h>

class TcpLink : public ILink
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TcpLink(QObject *parent = nullptr);
    ~TcpLink() override;

    quint16 port() const { return m_port; }
    bool isListening() const { return m_server.isListening(); }
    bool hasClient() const;

    Q_INVOKABLE bool listen(quint16 port) override;
    Q_INVOKABLE void close() override;
    Q_INVOKABLE bool sendTuning(double centerFrequencyHz, double spanHz, bool dfMode) override;

signals:


private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    void processBuffer();

    QTcpServer m_server;
    QTcpSocket *m_client = nullptr;
    QByteArray m_buffer;
    quint16 m_port = 0;
};