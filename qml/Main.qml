//Main.qml
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
    property bool lightMode: Application.styleHints.colorScheme === Qt.Light
    property color superDark: "#101010"
    property color reallyDark: "#1f1f1f"
    property color dark: "#262626"

    color: superDark

    TelemetryClient {
        id: telemetry
    }

    FontLoader {
        id: materialIcons
        source: "qrc:/qt/qml/faraz/resources/icons/MaterialSymbolsSharp.ttf"
    }

    RowLayout {
        anchors.fill: parent
        spacing: 5

        LeftMenu{}

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 5

                TopMenu {}

                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 400

                    MapContainer{}

                    // waterfall
                    Waterfall {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        source: telemetry
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
                        color: reallyDark
                        radius: 10
                        telemetry: telemetry
                    }

                    // Spectrum Wave
                    SpectrumWave {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        source: telemetry
                    }
                }
            }
        }
    }
}