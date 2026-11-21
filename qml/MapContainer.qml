import QtQuick
import QtQuick.Layouts
import QtLocation
import QtPositioning

Item {
    id: mapContainer
    property int cornerRadius: 10

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
        id: gisMap
        anchors.fill: parent
        plugin: mapPlugin
        activeMapType: supportedMapTypes[supportedMapTypes.length - 1]
        center: QtPositioning.coordinate(35.700956, 51.340737)
        zoomLevel: 16

        onCenterChanged: {
            console.log("Map center:", center.latitude, center.longitude)
        }

        // ring around the station
        MapCircle {
            center: df.station
            radius: 45
            color: "transparent"
            border.color: "#00b0ff"
            border.width: 2
        }

        // the one bearing line
        MapPolyline {
            visible: df.active && df.hasBearing
            line.width: 2
            line.color: "#00b0ff"
            path: [df.station, df.lineEnd]
        }

        // station marker
        MapQuickItem {
            coordinate: df.station
            anchorPoint.x: 7
            anchorPoint.y: 7
            sourceItem: Item {
                width: 14
                height: 14
                Rectangle {
                    anchors.fill: parent
                    rotation: 45
                    color: "#00b0ff"
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

        DragHandler {
            id: panHandler
            target: null
            acceptedButtons: Qt.LeftButton
            grabPermissions: PointerHandler.CanTakeOverFromItems | PointerHandler.ApprovesTakeOverByItems

            property real lastX: 0
            property real lastY: 0

            onActiveChanged: {
                if (active) {
                    lastX = translation.x
                    lastY = translation.y
                }
            }

            onTranslationChanged: {
                if (!active)
                    return
                var dx = translation.x - lastX
                var dy = translation.y - lastY
                lastX = translation.x
                lastY = translation.y
                // Dragging right/down moves the viewport left/up (-dx, -dy)
                gisMap.pan(-dx, -dy)
            }
        }
    }

    MapHeatmap {
        anchors.fill: parent
        map: gisMap
        locator: df
        enabled: false
    }

    Canvas {
        id: cornerRadiusOverlay
        anchors.fill: parent
        enabled: false
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        Connections {
            target: window
            function onSuperDarkChanged() { cornerRadiusOverlay.requestPaint() }
        }

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            var r = Math.min(mapContainer.cornerRadius, width / 2, height / 2)
            if (r <= 0)
                return

            ctx.fillStyle = window.superDark.toString()

            // Top-left corner
            ctx.beginPath()
            ctx.moveTo(0, 0)
            ctx.lineTo(r, 0)
            ctx.arc(r, r, r, -Math.PI / 2, Math.PI, true)
            ctx.closePath()
            ctx.fill()

            // Top-right corner
            ctx.beginPath()
            ctx.moveTo(width, 0)
            ctx.lineTo(width, r)
            ctx.arc(width - r, r, r, 0, -Math.PI / 2, true)
            ctx.closePath()
            ctx.fill()

            // Bottom-right corner
            ctx.beginPath()
            ctx.moveTo(width, height)
            ctx.lineTo(width - r, height)
            ctx.arc(width - r, height - r, r, Math.PI / 2, 0, true)
            ctx.closePath()
            ctx.fill()

            // Bottom-left corner
            ctx.beginPath()
            ctx.moveTo(0, height)
            ctx.lineTo(0, height - r)
            ctx.arc(r, height - r, r, Math.PI, Math.PI / 2, true)
            ctx.closePath()
            ctx.fill()
        }
    }
}