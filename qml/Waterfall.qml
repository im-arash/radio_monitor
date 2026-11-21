import QtQuick
import faraz

Rectangle {
    id: root
    color: reallyDark
    radius: 10
    border.width: 1
    border.color: "#333333"

    property alias source: wf.source
    property alias minDb: wf.minDb
    property alias maxDb: wf.maxDb

    WaterfallItem {
        id: wf
        anchors.fill: parent
        anchors.margins: 12
        anchors.leftMargin: 44
        anchors.bottomMargin: 28
    }
}