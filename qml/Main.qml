// Main.qml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtPositioning
import faraz

ApplicationWindow {
    id: window
    width: 1280
    height: 720
    minimumWidth: 200
    minimumHeight: 250
    visible: true
    title: qsTr("fc")

    color: Theme.backgroundColor

    StationModel {
        id: stationModel
        basePort: 9000
    }

    FontLoader {
        id: materialIcons
        source: "qrc:/qt/qml/faraz/resources/icons/MaterialSymbolsSharp.ttf"
    }

    SettingsPopup { id: settingsPopup }

    RowLayout {
        anchors.fill: parent
        spacing: 5

        LeftMenu {
            onSettingsRequested: settingsPopup.open()
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 5

                TopMenu {
                    model: stationModel
                    currentIndex: stationModel.currentIndex
                    onStationSelected: (index) => stationModel.currentIndex = index
                    onAddStationRequested: stationModel.addStation()
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 400

                    MapContainer {
                        stationState: stationModel.currentStation
                    }

                    Waterfall {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        source: stationModel.currentStation
                        minDb: -130
                        maxDb: -60
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 400

                    ResourceParameters {
                        Layout.preferredWidth: 800
                        Layout.fillHeight: true
                        radius: 10
                        stationState: stationModel.currentStation
                        onTuningRequested: (centerHz, spanHz, dfMode) =>
                            stationModel.tuneCurrentStation(centerHz, spanHz, dfMode)
                    }

                    SpectrumWave {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        source: stationModel.currentStation
                    }
                }
            }
        }
    }
}
