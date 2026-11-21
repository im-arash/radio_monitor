import QtQuick
import QtLocation
import faraz

HeatmapItem {
    id: root
    required property Map map

    function updateRect() {
        if (!locator) return
        const tl = map.fromCoordinate(locator.gridTopLeft, false)
        const br = map.fromCoordinate(locator.gridBottomRight, false)
        if (isNaN(tl.x) || isNaN(br.x)) return
        viewRect = Qt.rect(tl.x, tl.y, br.x - tl.x, br.y - tl.y)
    }

    // fromCoordinate() doesn't create binding dependencies, so refresh explicitly
    Connections {
        target: root.map
        function onCenterChanged()    { root.updateRect() }
        function onZoomLevelChanged() { root.updateRect() }
        function onWidthChanged()     { root.updateRect() }
        function onHeightChanged()    { root.updateRect() }
        function onMapReadyChanged()  { root.updateRect() }
    }
    Connections {
        target: root.locator
        function onGridChanged() { root.updateRect() }
    }

    onLocatorChanged: updateRect()
    Component.onCompleted: updateRect()
}