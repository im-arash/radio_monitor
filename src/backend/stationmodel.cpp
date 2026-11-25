// stationmodel.cpp
#include "stationmodel.h"

StationModel::StationModel(QObject *parent)
    : QAbstractListModel(parent)
{
    createInitialStations(1);
    if (!m_entries.isEmpty()) {
        m_currentIndex = 0;
    }
}

void StationModel::createInitialStations(int count)
{
    for (int i = 0; i < count; ++i) {
        addStation(QStringLiteral("Station %1").arg(i + 1));
    }
}

int StationModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_entries.size());
}

QVariant StationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return {};

    const Entry &entry = m_entries.at(index.row());
    switch (role) {
    case NameRole:
        return entry.state ? entry.state->name() : QString();
    case TcpPortRole:
        return entry.state ? entry.state->tcpPort() : 0;
    case StateRole:
        return entry.state ? QVariant::fromValue(entry.state) : QVariant();
    default:
        return {};
    }
}

QHash<int, QByteArray> StationModel::roleNames() const
{
    return {
        { NameRole, QByteArrayLiteral("name") },
        { TcpPortRole, QByteArrayLiteral("tcpPort") },
        { StateRole, QByteArrayLiteral("stationState") }
    };
}

void StationModel::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_entries.size() || index == m_currentIndex)
        return;

    m_currentIndex = index;
    emit currentIndexChanged();
    emit currentStationChanged();
}

StationState *StationModel::currentStation() const
{
    return stationAt(m_currentIndex);
}

StationState *StationModel::stationAt(int index) const
{
    if (index < 0 || index >= m_entries.size())
        return nullptr;
    return m_entries.at(index).state;
}

void StationModel::setBasePort(int port)
{
    if (port < 1 || port > 65535 || m_basePort == port)
        return;

    m_basePort = port;
    for (int i = 0; i < m_entries.size(); ++i) {
        const quint16 p = static_cast<quint16>(m_basePort + i);
        m_entries[i].state->setTcpPort(p);
        m_entries[i].stream->listen(p);
    }
    emit basePortChanged();
}

void StationModel::addStation(const QString &name)
{
    const int row = static_cast<int>(m_entries.size());
    const quint16 port = static_cast<quint16>(m_basePort + row);
    const QString resolvedName = name.isEmpty()
                                     ? QStringLiteral("Station %1").arg(row + 1)
                                     : name;

    beginInsertRows(QModelIndex(), row, row);

    auto *state = new StationState(this);
    state->setName(resolvedName);
    state->setTcpPort(port);

    auto *stream = new TcpLink(this);
    connect(stream, &TcpLink::packetReceived,
            state, &StationState::applyPacket);
    stream->listen(port);

    m_entries.push_back(Entry{ state, stream });

    endInsertRows();

    setCurrentIndex(row);
}

bool StationModel::tuneCurrentStation(double centerFrequencyHz, double spanHz, bool dfMode)
{
    return tuneStation(currentStation(), centerFrequencyHz, spanHz, dfMode);
}

bool StationModel::tuneStation(StationState *station, double centerFrequencyHz, double spanHz, bool dfMode)
{
    if (!station)
        return false;

    for (const Entry &entry : m_entries) {
        if (entry.state == station && entry.stream) {
            return entry.stream->sendTuning(centerFrequencyHz, spanHz, dfMode);
        }
    }
    return false;
}