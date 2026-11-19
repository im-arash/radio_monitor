import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    id: root

    // same palette as StationTabs
    readonly property color accent: "gray"
    readonly property color surface: "black"
    readonly property color surfaceHover: "#2e323c"
    readonly property color outline: "#555555"
    readonly property color textIdle: "#9a9a9a"

    color: "#202020"
    radius: 8
    border.width: 1
    border.color: "#333333"

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

            Slider {
                id: frequencySlider

                Layout.preferredWidth: 220
                Layout.preferredHeight: 55

                from: 0
                to: 6
                stepSize: 1
                snapMode: Slider.SnapAlways

                property var frequencies: [
                    "50 kHz", "100 kHz", "200 kHz",
                    "500 kHz", "1 MHz", "5 MHz", "10 MHz"
                ]

                background: Rectangle {
                    x: frequencySlider.leftPadding
                    y: frequencySlider.topPadding +
                       frequencySlider.availableHeight / 2 - height / 2

                    width: frequencySlider.availableWidth
                    height: 4
                    radius: 2
                    color: "#404040"

                    // filled part up to the handle
                    Rectangle {
                        width: frequencySlider.visualPosition *
                               (parent.width - frequencySlider.handle.width) +
                               frequencySlider.handle.width / 2
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
                            x: frequencySlider.handle.width / 2 +
                               index / 6 * (parent.width - frequencySlider.handle.width) - width / 2
                            y: (parent.height - height) / 2
                            z: -1
                        }
                    }
                }

                handle: Rectangle {
                    x: frequencySlider.leftPadding +
                       frequencySlider.visualPosition *
                       (frequencySlider.availableWidth - width)
                    y: frequencySlider.topPadding +
                       frequencySlider.availableHeight / 2 - height / 2

                    width: 12
                    height: 22
                    radius: 6
                    color: frequencySlider.pressed ? "#dddddd" : "white"
                    scale: frequencySlider.hovered || frequencySlider.pressed ? 1.1 : 1.0
                    Behavior on scale { NumberAnimation { duration: 120 } }
                }

                Text {
                    text: frequencySlider.frequencies[Math.round(frequencySlider.value)]
                    color: "white"
                    font.pixelSize: 12
                    font.bold: true

                    y: frequencySlider.topPadding +
                       frequencySlider.availableHeight / 2 + 17

                    x: frequencySlider.leftPadding +
                       frequencySlider.visualPosition *
                       (frequencySlider.availableWidth - frequencySlider.handle.width) +
                       frequencySlider.handle.width / 2 -
                       width / 2
                }
            }

            Switch {
                id: mySwitch
                text: "DF Mode"
                checked: false

                indicator: Rectangle {
                    implicitWidth: 42
                    implicitHeight: 22
                    x: mySwitch.leftPadding
                    y: parent.height / 2 - height / 2
                    radius: height / 2
                    color: mySwitch.checked ? root.accent : root.surface
                    border.width: 1
                    border.color: mySwitch.checked ? root.accent
                                : mySwitch.hovered ? "#888888"
                                : root.outline
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Behavior on border.color { ColorAnimation { duration: 150 } }

                    Rectangle {
                        width: 16
                        height: 16
                        radius: 8
                        color: "white"
                        y: (parent.height - height) / 2
                        x: 3 + mySwitch.visualPosition * (parent.width - width - 6)
                        Behavior on x { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }
                    }
                }

                contentItem: Text {
                    text: mySwitch.text
                    font.pixelSize: 14
                    font.bold: mySwitch.checked
                    color: mySwitch.checked ? "white" : root.textIdle
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: mySwitch.indicator.width + mySwitch.spacing
                    Behavior on color { ColorAnimation { duration: 150 } }
                }

                onToggled: {
                    console.log("Toggle:", checked)
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
