// ResourceParameters.qml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import faraz

Rectangle {
    id: root

    readonly property color accent: "gray"
    readonly property color surface: "black"
    readonly property color surfaceHover: "#2e323c"
    readonly property color outline: "#555555"
    readonly property color textIdle: "#9a9a9a"

    readonly property var unitFactors: [1, 1e3, 1e6, 1e9]
    readonly property var spans: [50e3, 100e3, 200e3, 500e3, 1e6, 5e6, 10e6]
    readonly property var spanLabels: ["50 kHz", "100 kHz", "200 kHz", "500 kHz", "1 MHz", "5 MHz", "10 MHz"]

    property var telemetry: null

    color: "#202020"
    radius: 8
    border.width: 1
    border.color: "#333333"

    // UI -> telemetry
    function applyFrequency() {
        if (!telemetry) return
        const value = parseFloat(freqField.text)
        if (isNaN(value)) return
        telemetry.centerFrequency = value * unitFactors[unitBox.currentIndex]
    }

    // telemetry -> UI (initial state & telemetry changes)
    function loadFromTelemetry() {
        if (!telemetry) return

        let idx = 2 // pick the largest unit that keeps value >= 1
        for (let i = unitFactors.length - 1; i >= 0; --i) {
            if (telemetry.centerFrequency >= unitFactors[i]) {
                idx = i
                break
            }
        }
        unitBox.currentIndex = idx
        freqField.text = String(telemetry.centerFrequency / unitFactors[idx])

        const s = spans.indexOf(telemetry.span)
        freqSlider.value = s >= 0 ? s : 4
        dfSwitch.checked = telemetry.dfMode
    }

    Component.onCompleted: loadFromTelemetry()
    onTelemetryChanged: loadFromTelemetry()

    RowLayout {
        anchors.fill: parent

        ColumnLayout {
            spacing: 14
            Layout.topMargin: 14
            Layout.leftMargin: 14

            Label {
                text: "Frequency:"
                font.pixelSize: 14
                font.bold: true
                color: "white"
            }

            Row {
                spacing: 6

                TextField {
                    id: freqField
                    width: 120
                    height: 32
                    leftPadding: 10
                    rightPadding: 10
                    color: "white"
                    font.pixelSize: 14
                    placeholderText: "Value"
                    placeholderTextColor: "#666666"
                    selectByMouse: true
                    verticalAlignment: TextInput.AlignVCenter
                    validator: DoubleValidator { bottom: 0 }

                    onEditingFinished: root.applyFrequency()

                    background: Rectangle {
                        radius: 8
                        color: freqField.hovered || freqField.activeFocus ? root.surfaceHover : root.surface
                        border.width: 1
                        border.color: freqField.activeFocus ? "white"
                                    : freqField.hovered ? "#888888"
                                    : root.outline
                        Behavior on color { ColorAnimation { duration: 150 } }
                        Behavior on border.color { ColorAnimation { duration: 150 } }
                    }
                }

                ComboBox {
                    id: unitBox
                    width: 80
                    height: 32
                    model: ["Hz", "kHz", "MHz", "GHz"]
                    font.pixelSize: 14

                    onActivated: root.applyFrequency()

                    contentItem: Text {
                        leftPadding: 10
                        text: unitBox.displayText
                        font: unitBox.font
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
                    }

                    indicator: Text {
                        x: unitBox.width - width - 10
                        y: (unitBox.height - height) / 2
                        text: "\u25BE"
                        font.pixelSize: 12
                        color: unitBox.hovered ? "white" : root.textIdle
                        rotation: unitBox.popup.visible ? 180 : 0
                        Behavior on rotation { NumberAnimation { duration: 150 } }
                    }

                    background: Rectangle {
                        radius: 8
                        color: unitBox.hovered || unitBox.popup.visible ? root.surfaceHover : root.surface
                        border.width: 1
                        border.color: unitBox.popup.visible ? "white"
                                    : unitBox.hovered ? "#888888"
                                    : root.outline
                        Behavior on color { ColorAnimation { duration: 150 } }
                        Behavior on border.color { ColorAnimation { duration: 150 } }
                    }

                    delegate: ItemDelegate {
                        id: unitItem
                        required property var modelData
                        required property int index
                        width: unitBox.width - 8
                        height: 30
                        highlighted: unitBox.highlightedIndex === index

                        contentItem: Text {
                            text: unitItem.modelData
                            font: unitBox.font
                            color: unitItem.highlighted ? "white" : root.textIdle
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            radius: 6
                            color: unitItem.highlighted ? "#3a3f4b" : "transparent"
                        }
                    }

                    popup: Popup {
                        y: unitBox.height + 4
                        width: unitBox.width
                        padding: 4
                        implicitHeight: contentItem.implicitHeight + 8

                        contentItem: ListView {
                            clip: true
                            implicitHeight: contentHeight
                            model: unitBox.popup.visible ? unitBox.delegateModel : null
                            currentIndex: unitBox.highlightedIndex
                        }
                        background: Rectangle {
                            radius: 8
                            color: root.surface
                            border.width: 1
                            border.color: "#444444"
                        }
                    }
                }
            }

            Label {
                text: "Span:"
                font.pixelSize: 14
                font.bold: true
                color: "white"
            }

            Slider {
                id: freqSlider

                Layout.preferredWidth: 220
                Layout.preferredHeight: 55

                from: 0
                to: 6
                stepSize: 1
                snapMode: Slider.SnapAlways

                onMoved: {
                    if (root.telemetry)
                        root.telemetry.span = root.spans[Math.round(value)]
                }

                background: Rectangle {
                    x: freqSlider.leftPadding
                    y: freqSlider.topPadding +
                       freqSlider.availableHeight / 2 - height / 2

                    width: freqSlider.availableWidth
                    height: 4
                    radius: 2
                    color: "#404040"

                    // filled part up to the handle
                    Rectangle {
                        width: freqSlider.visualPosition *
                               (parent.width - freqSlider.handle.width) +
                               freqSlider.handle.width / 2
                        height: parent.height
                        radius: 2
                        color: root.accent
                    }

                    // one tick per step
                    Repeater {
                        model: 7
                        Rectangle {
                            required property int index
                            width: 2
                            height: 10
                            radius: 1
                            color: "#555555"
                            x: freqSlider.handle.width / 2 +
                               index / 6 * (parent.width - freqSlider.handle.width) - width / 2
                            y: (parent.height - height) / 2
                            z: -1
                        }
                    }
                }

                handle: Rectangle {
                    x: freqSlider.leftPadding +
                       freqSlider.visualPosition *
                       (freqSlider.availableWidth - width)
                    y: freqSlider.topPadding +
                       freqSlider.availableHeight / 2 - height / 2

                    width: 12
                    height: 22
                    radius: 6
                    color: freqSlider.pressed ? "#dddddd" : "white"
                    scale: freqSlider.hovered || freqSlider.pressed ? 1.1 : 1.0
                    Behavior on scale { NumberAnimation { duration: 120 } }
                }

                Text {
                    text: root.spanLabels[Math.round(freqSlider.value)]
                    color: "white"
                    font.pixelSize: 12
                    font.bold: true

                    y: freqSlider.topPadding +
                       freqSlider.availableHeight / 2 + 17

                    x: Math.max(
                        freqSlider.leftPadding,
                        Math.min(
                            freqSlider.leftPadding + freqSlider.availableWidth - width,
                            freqSlider.leftPadding +
                            freqSlider.visualPosition *
                            (freqSlider.availableWidth - freqSlider.handle.width) +
                            freqSlider.handle.width / 2 -
                            width / 2
                        )
                    )
                }
            }

            Switch {
                id: dfSwitch
                text: "DF Mode"
                checked: false

                onToggled: {
                    if (root.telemetry)
                        root.telemetry.dfMode = checked
                }

                indicator: Rectangle {
                    implicitWidth: 42
                    implicitHeight: 22
                    x: dfSwitch.leftPadding
                    y: parent.height / 2 - height / 2
                    radius: height / 2
                    color: dfSwitch.checked ? root.accent : root.surface
                    border.width: 1
                    border.color: dfSwitch.checked ? root.accent
                                : dfSwitch.hovered ? "#888888"
                                : root.outline
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Behavior on border.color { ColorAnimation { duration: 150 } }

                    Rectangle {
                        width: 16
                        height: 16
                        radius: 8
                        color: "white"
                        y: (parent.height - height) / 2
                        x: 3 + dfSwitch.visualPosition * (parent.width - width - 6)
                        Behavior on x { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }
                    }
                }

                contentItem: Text {
                    text: dfSwitch.text
                    font.pixelSize: 14
                    font.bold: dfSwitch.checked
                    color: dfSwitch.checked ? "white" : root.textIdle
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: dfSwitch.indicator.width + dfSwitch.spacing
                    Behavior on color { ColorAnimation { duration: 150 } }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }
}