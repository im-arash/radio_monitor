import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtLocation
import QtPositioning


ApplicationWindow {
    id: window
    width: 1280
    height: 720
    minimumWidth: 200
    minimumHeight: 250
    visible: true
    title: qsTr("farazcomm")
    property bool lightMode: Application.styleHints.colorScheme === Qt.Light
    property color reallyDark: "#1f1f1f"
    property color dark: "#262626"

    color: reallyDark

    RowLayout{
        anchors.fill: parent

        ColumnLayout{
            id: leftMenu
            Layout.preferredWidth: 60
            Layout.fillHeight: true
            spacing: 20

            Item{
                Layout.fillHeight: true
            }

            Image {
                id: name
                Layout.preferredWidth: 20
                Layout.preferredHeight: 20

                source: "resources/images/farazlogo.png"
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 20
            }
        }

        Item{
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout{
                anchors.fill: parent

                TopMenu{}

                RowLayout{
                    Layout.fillWidth: true
                    Layout.preferredHeight: 400
                    Item {
                        Layout.preferredWidth: 800
                        Layout.preferredHeight: 400

                        Plugin {
                            id: mapPlugin
                            name: "osm"

                            PluginParameter {
                                name: "osm.mapping.custom.host"
                                value: "https://tile.openstreetmap.org/"
                            }
                        }

                        Map {
                            anchors.fill: parent
                            plugin: mapPlugin

                            activeMapType: supportedMapTypes[supportedMapTypes.length - 1]

                            center: QtPositioning.coordinate(35.700956, 51.340737) // Baku
                            zoomLevel: 16

                            MapQuickItem {
                                coordinate: QtPositioning.coordinate(35.700956, 51.340737)

                                anchorPoint.x: marker.width / 2
                                anchorPoint.y: marker.height

                                sourceItem: Rectangle {
                                    id: marker
                                    width: 16
                                    height: 16
                                    radius: 8
                                    color: "blue"
                                    border.color: "white"
                                    border.width: 2
                                }
                            }
                        }
                    }

                    Rectangle{
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 10
                        color: "red"
                    }
                }

                RowLayout{
                    Layout.fillWidth: true
                    Layout.preferredHeight: 400
                    ResourceParameters{
                        Layout.preferredWidth: 800
                        Layout.fillHeight: true
                        color: dark
                        radius: 10
                    }

                    Rectangle{
                        color: "blue"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 10
                    }
                }

            }
        }
    }
}
