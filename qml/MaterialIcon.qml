//MaterialIcon.qml
import QtQuick

Item {
    property string icon
    property int size: 28
    property color iconColor: "gray"

    implicitWidth: size
    implicitHeight: size

    anchors.centerIn: parent
    Text {
        anchors.centerIn: parent
        text: icon
        font.family: materialIcons.name
        font.pixelSize: size
        color: iconColor
    }
}