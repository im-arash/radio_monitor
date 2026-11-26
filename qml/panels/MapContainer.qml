import QtQuick
import QtQuick.Layouts
import QtQuick.Effects
import QtLocation
import QtPositioning
import faraz

Item {
    id: mapContainer
    property int cornerRadius: 10
    property var stationState: null
    readonly property var df: stationState

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
        copyrightsVisible: false

        // false = your custom OSM tiles (always the last type)
        // true  = the plugin's built-in night map type, if it exists
        function pickMapType(night) {
            const types = supportedMapTypes
            if (!types || types.length === 0)
                return undefined
            if (night) {
                for (let i = 0; i < types.length; ++i) {
                    if (types[i].night)
                        return types[i]
                }
            }
            return types[types.length - 1]
        }
        activeMapType: pickMapType(Theme.mapNightTiles)

        // "light" and "dark" show the provider's tiles as they are.
        // "theme" makes the tiles black & white first, then the tint below adds the background color.
        // Only the tiles are affected: the DF marker, circle and bearing line are drawn by
        // MapOverlay above the map, and the heatmap is a sibling item.
        layer.enabled: Theme.mapStyle === "theme"
        layer.effect: MultiEffect {
            saturation: -1.0      // optional: brightness / contrast (-1..1)
        }

        center: (df && df.station && df.station.isValid)
                ? df.station
                : QtPositioning.coordinate(0, 0)
        zoomLevel: 16

        WheelHandler {
                    id: wheelZoom
                    acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
                    rotationScale: 1 / 120
                    property: "zoomLevel"
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

    // "Match theme": tint the gray map with the app background color.
    // Sits above the map and below the overlay, so marker/line/circle stay untouched.
    Rectangle {
        anchors.fill: parent
        visible: Theme.mapStyle === "theme"
        color: Theme.backgroundColor
        opacity: 0.4              // tint strength: 0 = plain gray map, 1 = solid background color
        enabled: false
    }

    MapOverlay {
        anchors.fill: parent
        map: gisMap
        df: mapContainer.df
        enabled: false
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
            target: Theme
            function onBackgroundColorChanged() { cornerRadiusOverlay.requestPaint() }
        }

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            var r = Math.min(mapContainer.cornerRadius, width / 2, height / 2)
            if (r <= 0)
                return

            ctx.fillStyle = Theme.backgroundColor.toString()

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
