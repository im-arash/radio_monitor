import QtQuick
import QtQuick.Layouts

Item {
    id: root
    Layout.preferredHeight: 40
    Layout.fillWidth: true

    property int currentIndex: 0
    readonly property color accent: "gray"

    signal stationAdded(int index)

    ListModel {
        id: stations
        ListElement { name: "Station 1" }
        ListElement { name: "Station 2" }
        ListElement { name: "Station 3" }
    }

    RowLayout {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 8
        spacing: 4

        Repeater {
            model: stations

            Rectangle {
                id: tab
                required property int index
                required property string name
                readonly property bool selected: root.currentIndex === index

                Layout.preferredWidth: label.implicitWidth + 36
                Layout.preferredHeight: 32
                radius: 8
                color: selected ? "black"
                     : tabMouse.containsMouse ? "#2a2a2a"
                     : "transparent"
                Behavior on color { ColorAnimation { duration: 150 } }

                Text {
                    id: label
                    anchors.centerIn: parent
                    text: tab.name
                    font.pixelSize: 14
                    font.bold: tab.selected
                    color: tab.selected ? "white"
                         : tabMouse.containsMouse ? "#dddddd"
                         : "#9a9a9a"
                    Behavior on color { ColorAnimation { duration: 150 } }
                }

                // accent underline that grows in when selected
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
                    onClicked: root.currentIndex = tab.index
                }
            }
        }

        // spacer between tabs and the add button
        Item { Layout.preferredWidth: 6 }

        // "New" button
        Rectangle {
            id: addBtn
            Layout.preferredWidth: addRow.implicitWidth + 24
            Layout.preferredHeight: 28
            Layout.alignment: Qt.AlignVCenter
            radius: height / 2
            color: addMouse.containsMouse ? "black" : "transparent"
            border.width: 1
            border.color: addMouse.containsMouse ? root.accent : "#555555"
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
                    color: addMouse.containsMouse ? "white" : "#9a9a9a"
                    anchors.verticalCenter: parent.verticalCenter
                    rotation: addMouse.containsMouse ? 90 : 0
                    Behavior on rotation { NumberAnimation { duration: 200 } }
                }
                Text {
                    text: "New"
                    font.pixelSize: 13
                    color: addMouse.containsMouse ? "white" : "#9a9a9a"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            MouseArea {
                id: addMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    stations.append({ name: "Station " + (stations.count + 1) })
                    root.currentIndex = stations.count - 1
                    root.stationAdded(root.currentIndex)
                }
            }
        }
    }

    // thin separator under the whole bar
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: "#333333"
    }
}
