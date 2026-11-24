import QtQuick
import QtLocation
import faraz

HeatmapItem {
    id: root
    required property Map map

    function updateRect() {
        if (!map) return
        const tl = map.fromCoordinate(root.gridTopLeft, false)
        const br = map.fromCoordinate(root.gridBottomRight, false)
        if (isNaN(tl.x) || isNaN(br.x)) return
        viewRect = Qt.rect(tl.x, tl.y, br.x - tl.x, br.y - tl.y)
    }

    Connections {
        target: root.map
        function onCenterChanged()    { root.updateRect() }
        function onZoomLevelChanged() { root.updateRect() }
        function onWidthChanged()     { root.updateRect() }
        function onHeightChanged()    { root.updateRect() }
        function onMapReadyChanged()  { root.updateRect() }
    }

    onGridChanged: updateRect()
    Component.onCompleted: updateRect()
}