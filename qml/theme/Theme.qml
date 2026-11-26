// Theme.qml  -- single source of truth for every color in the app.
pragma Singleton
import QtQuick
import QtCore

QtObject {
    id: root

    // ------------------------------------------------------------------
    // 1) USER-EDITABLE (persisted automatically)
    // ------------------------------------------------------------------
    property color backgroundColor: "#101010"
    property color accentColor:     "#00b0ff"
    property color iconColor:       "#808080"
    property string mapStyle: "light"     // "light" | "dark" | "theme"

    // which tile set the map uses: the night type or the light OSM type
    readonly property bool mapNightTiles: mapStyle === "dark"
                                          || (mapStyle === "theme" && isDark)

    // ------------------------------------------------------------------
    // 2) DERIVED from the three above (never edited directly)
    // ------------------------------------------------------------------
    function mix(a, b, t) {
        return Qt.rgba(a.r + (b.r - a.r) * t,
                       a.g + (b.g - a.g) * t,
                       a.b + (b.b - a.b) * t, 1)
    }

    readonly property bool  isDark: (0.299 * backgroundColor.r
                                   + 0.587 * backgroundColor.g
                                   + 0.114 * backgroundColor.b) < 0.5

    readonly property color textPrimary:  isDark ? "#ffffff" : "#121212"
    readonly property color inputColor:   isDark ? "#000000" : "#ffffff"

    readonly property color panelColor:   mix(backgroundColor, textPrimary, 0.065) // cards / panels
    readonly property color borderColor:  mix(backgroundColor, textPrimary, 0.14)
    readonly property color gridColor:    mix(backgroundColor, textPrimary, 0.10)
    readonly property color hoverColor:   mix(backgroundColor, textPrimary, 0.15)
    readonly property color grooveColor:  mix(backgroundColor, textPrimary, 0.20)
    readonly property color outlineColor: mix(backgroundColor, textPrimary, 0.28)
    readonly property color outlineHover: mix(backgroundColor, textPrimary, 0.50)
    readonly property color textDim:      mix(backgroundColor, textPrimary, 0.50)
    readonly property color textMuted:    mix(backgroundColor, textPrimary, 0.60)
    readonly property color textSoft:     mix(backgroundColor, textPrimary, 0.80)

    // fixed semantic color (not themed)
    readonly property color warningColor: "#d05016"

    // ------------------------------------------------------------------
    // 3) Presets + reset
    // ------------------------------------------------------------------
    readonly property var presets: [
        { name: "Dark",     background: "#101010", accent: "#808080", icon: "#808080" },
        { name: "Midnight", background: "#0b1220", accent: "#4dabf7", icon: "#8aa4c8" },
        { name: "Emerald",  background: "#0d1512", accent: "#2ecc71", icon: "#7fa08f" },
        { name: "Light",    background: "#f2f2f2", accent: "#0078d4", icon: "#5f6368" }
    ]

    function applyPreset(p) {
        backgroundColor = p.background
        accentColor     = p.accent
        iconColor       = p.icon
    }

    function resetAll() { applyPreset(presets[0]); mapStyle = "light" }

    // ------------------------------------------------------------------
    // 4) Persistence (needs organizationName/applicationName in main.cpp)
    // ------------------------------------------------------------------
    property Settings store: Settings {
        category: "theme"
        property alias background: root.backgroundColor
        property alias accent:     root.accentColor
        property alias icon:       root.iconColor
        property alias map: root.mapStyle
    }
}
