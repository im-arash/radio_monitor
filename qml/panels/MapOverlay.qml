import QtQuick
import QtLocation
import QtPositioning
import faraz

// DF marker, accuracy circle and bearing line.
// Drawn ABOVE the map (not inside it), so any effect applied to the map tiles
// (e.g. the grayscale dark map) never changes how these look.
Item {
    id: root
    clip: true

    required property Map map
    required property var df            // StationState

    readonly property bool hasStation: !!(df && df.station && df.station.isValid)
    readonly property bool hasLine: !!(hasStation && df.active && df.hasBearing
                                       && df.lineEnd && df.lineEnd.isValid)

    // bumped whenever the map view changes, so the bindings below are re-evaluated
    property int tick: 0

    readonly property point stationPt: {
        const t = tick
        return hasStation ? map.fromCoordinate(df.station, false) : Qt.point(0, 0)
    }
    readonly property point endPt: {
        const t = tick
        return hasLine ? map.fromCoordinate(df.lineEnd, false) : Qt.point(0, 0)
    }
    // 45 m circle, converted to pixels at the current zoom
    readonly property real circleRadiusPx: {
        const t = tick
        if (!hasStation)
            return 0
        const edge = map.fromCoordinate(df.station.atDistanceAndAzimuth(45, 90), false)
        return Math.abs(edge.x - stationPt.x)
    }
    readonly property real lineDx: endPt.x - stationPt.x
    readonly property real lineDy: endPt.y - stationPt.y

    Connections {
        target: root.map
        function onCenterChanged()    { root.tick++ }
        function onZoomLevelChanged() { root.tick++ }
        function onWidthChanged()     { root.tick++ }
        function onHeightChanged()    { root.tick++ }
        function onMapReadyChanged()  { root.tick++ }
    }
    onMapChanged: tick++

    // accuracy circle
    Rectangle {
        visible: root.hasStation
        x: root.stationPt.x - width / 2
        y: root.stationPt.y - height / 2
        width: root.circleRadiusPx * 2
        height: width
        radius: width / 2
        color: "transparent"
        border.color: Theme.accentColor
        border.width: 2
        antialiasing: true
    }

    // bearing line
    Rectangle {
        visible: root.hasLine
        x: root.stationPt.x
        y: root.stationPt.y - height / 2
        width: Math.hypot(root.lineDx, root.lineDy)
        height: 2
        color: Theme.accentColor
        transformOrigin: Item.Left
        rotation: Math.atan2(root.lineDy, root.lineDx) * 180 / Math.PI
        antialiasing: true
    }

    // DF marker
    Item {
        visible: root.hasStation
        x: root.stationPt.x - 7
        y: root.stationPt.y - 7
        width: 14
        height: 14

        Rectangle {
            anchors.fill: parent
            rotation: 45
            color: Theme.accentColor
            border.color: "white"
            border.width: 2
        }
        Text {
            x: 18
            y: -2
            text: "DF 1"
            color: "white"
            font.pixelSize: 11
            style: Text.Outline
            styleColor: "black"
        }
    }
}
