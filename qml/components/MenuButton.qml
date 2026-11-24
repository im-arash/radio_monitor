//MenuButton.qml
import QtQuick
import QtQuick.Layouts
import faraz

Rectangle {
    id: menuBtn
    property string iconName
    property color iconColor: Theme.iconColor
    property int iconSize: 24
    signal clicked()

    height: 20
    width: 20
    radius: 10
    Layout.alignment: Qt.AlignHCenter
    color: "transparent"

    MaterialIcon {
            anchors.centerIn: parent
            icon: menuBtn.iconName
            iconColor: menuBtn.iconColor
            size: menuBtn.iconSize
        }

    HoverHandler {
        // onHoveredChanged: parent.color = hovered ? "#606060" : "transparent"
    }

    TapHandler {
        onTapped: parent.clicked()
    }
}
