// tcpclient.h
#pragma once

#include "packet.h"

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QHostAddress>
#include <QtQml/qqmlregistration.h>

class TcpClient : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TcpClient(QObject *parent = nullptr);
    ~TcpClient() override;

    Q_INVOKABLE bool listen(quint16 port);
    Q_INVOKABLE void close();
    quint16 port() const { return m_port; }
    bool isListening() const { return m_server.isListening(); }
    bool hasClient() const;

    Q_INVOKABLE bool sendTuning(double centerFrequencyHz, double spanHz, bool dfMode);

signals:
    void packetReceived(const faraz::Packet &packet);
    void clientConnected();
    void clientDisconnected();

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