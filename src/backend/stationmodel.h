// stationmodel.h
#pragma once

#include "stationstate.h"
#include "ilink.h"

#include <QAbstractListModel>
#include <QList>
#include <QtQml/qqmlregistration.h>

class StationModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(StationState *currentStation READ currentStation NOTIFY currentStationChanged)
    Q_PROPERTY(int basePort READ basePort WRITE setBasePort NOTIFY basePortChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        TcpPortRole,
        StateRole
    };

    explicit StationModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int currentIndex() const { return m_currentIndex; }
    void setCurrentIndex(int index);

    StationState *currentStation() const;
    Q_INVOKABLE StationState *stationAt(int index) const;

    int basePort() const { return m_basePort; }
    void setBasePort(int port);

    Q_INVOKABLE void addStation(const QString &name = QString());

    Q_INVOKABLE bool tuneCurrentStation(double centerFrequencyHz, double spanHz, bool dfMode);
    Q_INVOKABLE bool tuneStation(StationState *station, double centerFrequencyHz, double spanHz, bool dfMode);

signals:
    void currentIndexChanged();
    void currentStationChanged();
    void basePortChanged();

private:
    struct Entry {
        StationState *state = nullptr;
        ILink *stream = nullptr;
    };

    void createInitialStations(int count);

    QList<Entry> m_entries;
    int m_currentIndex = -1;
    int m_basePort = 9000;
    ILink *createLink();
};