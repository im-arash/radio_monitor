import QtQuick
import faraz

Rectangle {
    id: root
    color: Theme.panelColor
    radius: 10
    border.width: 1
    border.color: Theme.borderColor

    property alias minDb: wave.minDb
    property alias maxDb: wave.maxDb
    property alias source: wave.source
    property alias wave: wave

    function formatHz(hz) {
        const a = Math.abs(hz)
        if (a >= 1e9) return (hz / 1e9).toFixed(4) + " GHz"
        if (a >= 1e6) return (hz / 1e6).toFixed(4) + " MHz"
        if (a >= 1e3) return (hz / 1e3).toFixed(2) + " kHz"
        return hz.toFixed(0) + " Hz"
    }

    Item {
        id: plot
        anchors.fill: parent
        anchors.margins: 12
        anchors.leftMargin: 44
        anchors.bottomMargin: 28

        // horizontal grid + dB labels (every 10 dB)
        Repeater {
            model: Math.round((root.maxDb - root.minDb) / 10) + 1
            Item {
                required property int index
                width: plot.width
                y: plot.height * index / (model_count - 1)
                readonly property int model_count: Math.round((root.maxDb - root.minDb) / 10) + 1

                Rectangle { width: parent.width; height: 1; color: Theme.gridColor }
                Text {
                    x: -40; y: -height / 2
                    text: Math.round(root.maxDb - index * 10)
                    color: Theme.textDim
                    font.pixelSize: 10
                }
            }
        }

        // vertical grid (10 divisions)
        Repeater {
            model: 11
            Rectangle {
                required property int index
                x: plot.width * index / 10
                width: 1; height: plot.height
                color: Theme.gridColor
            }
        }

        SpectrumWaveItem {
                    id: wave
                    anchors.fill: parent
                    minDb: -140
                    maxDb: -40
                    traceColor: Theme.accentColor
                }

                // frequency axis
                Row {
                    y: plot.height + 6
                    width: plot.width
                    property real c: root.source ? root.source.centerFrequency : 0
                    property real s: root.source ? root.source.span : 0

                    Text {
                        width: parent.width / 3
                        text: root.formatHz(parent.c - parent.s / 2)
                        color: Theme.textDim; font.pixelSize: 10
                    }
                    Text {
                        width: parent.width / 3
                        horizontalAlignment: Text.AlignHCenter
                        text: root.formatHz(parent.c)
                        color: Theme.textSoft; font.pixelSize: 10
                    }
                    Text {
                        width: parent.width / 3
                        horizontalAlignment: Text.AlignRight
                        text: root.formatHz(parent.c + parent.s / 2)
                        color: Theme.textDim; font.pixelSize: 10
                    }
                }
    }
}
