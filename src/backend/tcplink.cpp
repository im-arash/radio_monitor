// tcplink.cpp
#include "tcplink.h"

#include <cstddef>
#include <cstring>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
constexpr quint32 kMaxTcpFrameBytes = 1024 * 1024;
} // namespace

TcpLink::TcpLink(QObject *parent)
    : QObject(parent)
{
    connect(&m_server, &QTcpServer::newConnection,
            this, &TcpLink::onNewConnection);
}

TcpLink::~TcpLink()
{
    close();
}

bool TcpLink::listen(quint16 port)
{
    close();
    m_port = port;

    if (m_server.listen(QHostAddress::AnyIPv4, m_port)) {
        qInfo() << "TcpLink listening on port" << m_port;
        return true;
    }

    qWarning() << "TcpLink failed to listen on port" << m_port << m_server.errorString();
    return false;
}

void TcpLink::close()
{
    if (m_client) {
        m_client->disconnect(this);
        m_client->disconnectFromHost();
        m_client->abort();
        m_client->deleteLater();
        m_client = nullptr;
    }
    m_buffer.clear();
    m_server.close();
}

bool TcpLink::hasClient() const
{
    return m_client && m_client->state() == QAbstractSocket::ConnectedState;
}

void TcpLink::onNewConnection()
{
    QTcpSocket *next = m_server.nextPendingConnection();
    if (!next)
        return;

    const bool hadClient = (m_client != nullptr);
    if (m_client) {
        m_client->disconnect(this);
        m_client->abort();
        m_client->deleteLater();
        m_client = nullptr;
        m_buffer.clear();
    }

    m_client = next;
    m_client->setSocketOption(QAbstractSocket::LowDelayOption, 1);

    connect(m_client, &QTcpSocket::readyRead,
            this, &TcpLink::onReadyRead);
    connect(m_client, &QTcpSocket::disconnected,
            this, &TcpLink::onDisconnected);

    qInfo() << "TcpLink accepted connection from"
            << m_client->peerAddress().toString() << m_client->peerPort();

    if (!hadClient) {
        emit clientConnected();
    }
}

void TcpLink::onDisconnected()
{
    if (!m_client)
        return;

    m_client->deleteLater();
    m_client = nullptr;
    m_buffer.clear();
    emit clientDisconnected();
}

void TcpLink::onReadyRead()
{
    if (!m_client)
        return;

    m_buffer.append(m_client->readAll());
    processBuffer();
}

void TcpLink::processBuffer()
{
    constexpr int headerBytes = static_cast<int>(sizeof(quint32));
    constexpr quint32 headerOnlyBytes = static_cast<quint32>(offsetof(faraz::Packet, bins));

    while (m_buffer.size() >= headerBytes) {
        const auto *raw = reinterpret_cast<const uchar *>(m_buffer.constData());
        const quint32 frameSize = (static_cast<quint32>(raw[0]) << 24)
                                  | (static_cast<quint32>(raw[1]) << 16)
                                  | (static_cast<quint32>(raw[2]) << 8)
                                  | (static_cast<quint32>(raw[3]));

        if (frameSize == 0 || frameSize > kMaxTcpFrameBytes) {
            qWarning() << "Invalid TCP frame size:" << frameSize << "- clearing buffer";
            m_buffer.clear();
            return;
        }

        const int totalNeeded = headerBytes + static_cast<int>(frameSize);
        if (m_buffer.size() < totalNeeded) {
            return;
        }

        if (frameSize >= headerOnlyBytes && frameSize <= sizeof(faraz::Packet)) {
            faraz::Packet pkt{};
            std::memcpy(&pkt, raw + headerBytes, static_cast<size_t>(frameSize));

            const quint32 maxPossibleBins =
                (frameSize - headerOnlyBytes) / static_cast<quint32>(sizeof(float));
            if (pkt.binCount > maxPossibleBins) {
                pkt.binCount = maxPossibleBins;
            }

            if (pkt.magic == faraz::kPacketMagic && pkt.version == faraz::kPacketVersion) {
                emit packetReceived(pkt);
            } else {
                qWarning() << "Invalid Packet magic/version";
            }
        } else {
            qWarning() << "Unexpected Packet payload size:" << frameSize;
        }

        m_buffer.remove(0, totalNeeded);
    }
}

bool TcpLink::sendTuning(double centerFrequencyHz, double spanHz, bool dfMode)
{
    if (!hasClient()) {
        qWarning() << "TcpLink::sendTuning: no connected client on port" << m_port;
        return false;
    }

    QJsonObject obj;
    obj.insert(QStringLiteral("type"), QStringLiteral("tune"));
    obj.insert(QStringLiteral("centerFrequencyHz"), centerFrequencyHz);
    obj.insert(QStringLiteral("spanHz"), spanHz);
    obj.insert(QStringLiteral("dfMode"), dfMode);

    QByteArray line = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    line.append('\n');

    const qint64 written = m_client->write(line);
    return written == line.size();
}