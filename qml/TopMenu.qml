// TopMenu.qml
import QtQuick
import QtQuick.Layouts
import faraz

Item {
    id: root
    Layout.preferredHeight: 40
    Layout.fillWidth: true

    property alias model: repeater.model
    property int currentIndex: 0
    readonly property color accent: Theme.accentColor

    signal stationSelected(int index)
    signal addStationRequested()

    RowLayout {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        spacing: 4

        Repeater {
            id: repeater

            Rectangle {
                id: tab
                required property int index
                required property string name
                readonly property bool selected: root.currentIndex === index

                Layout.preferredWidth: label.implicitWidth + 36
                Layout.preferredHeight: 32
                radius: 8
                color: selected ? Theme.inputColor
                     : tabMouse.containsMouse ? Theme.hoverColor
                     : "transparent"
                Behavior on color { ColorAnimation { duration: 150 } }

                Text {
                    id: label
                    anchors.centerIn: parent
                    text: tab.name
                    font.pixelSize: 14
                    font.bold: tab.selected
                    color: tab.selected ? Theme.textPrimary
                         : tabMouse.containsMouse ? Theme.textSoft
                         : Theme.textMuted
                    Behavior on color { ColorAnimation { duration: 150 } }
                }

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    height: 2
                    radius: 1
                    color: root.accent
                    width: tab.selected ? parent.width - 16 : 0
                    Behavior on width { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }
                }

                MouseArea {
                    id: tabMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.stationSelected(tab.index)
                }
            }
        }

        Item { Layout.preferredWidth: 6 }

        Rectangle {
            id: addBtn
            Layout.preferredWidth: addRow.implicitWidth + 24
            Layout.preferredHeight: 28
            Layout.alignment: Qt.AlignVCenter
            radius: height / 2
            color: addMouse.containsMouse ? Theme.inputColor : "transparent"
            border.width: 1
            border.color: addMouse.containsMouse ? root.accent : Theme.outlineColor
            Behavior on color { ColorAnimation { duration: 150 } }
            Behavior on border.color { ColorAnimation { duration: 150 } }

            Row {
                id: addRow
                anchors.centerIn: parent
                spacing: 6

                Text {
                    id: plus
                    text: "+"
                    font.pixelSize: 16
                    font.bold: true
                    color: addMouse.containsMouse ? Theme.textPrimary : Theme.textMuted
                    anchors.verticalCenter: parent.verticalCenter
                    rotation: addMouse.containsMouse ? 90 : 0
                    Behavior on rotation { NumberAnimation { duration: 200 } }
                }
                Text {
                    text: "New"
                    font.pixelSize: 13
                    color: addMouse.containsMouse ? Theme.textPrimary : Theme.textMuted
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            MouseArea {
                id: addMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.addStationRequested()
            }
        }

        Item {
            Layout.fillWidth: true
        }

        MenuButton {
            iconName: "warning"
            iconColor: Theme.warningColor
            onClicked: console.log("clicked search")
        }
        Text {
            text: "Threat Detected"
            color: Theme.warningColor
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: Theme.borderColor
    }
}
