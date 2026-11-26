// SettingsPopup.qml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import faraz

Popup {
    id: root

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: 460
    modal: true
    focus: true
    padding: 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        radius: 10
        color: Theme.panelColor
        border.width: 1
        border.color: Theme.borderColor
    }

    Overlay.modal: Rectangle { color: Qt.rgba(0, 0, 0, 0.5) }

    // -------- reusable color editor row --------
    component ColorRow: ColumnLayout {
        id: row
        property string label
        property color value
        signal picked(color c)

        readonly property var swatches: [
            // Black
            "#101010",

            // Gray
            "#808080", "#5f6368",

            // White
            "#f2f2f2",

            // Dark blue
            "#0b1220","#1a1d29",

            // Blue
            "#00b0ff", "#4dabf7", "#8aa4c8",

            // Dark green
            "#0d1512",

            // Green
            "#2ecc71", "#7fa08f",

            // Orange
            "#f5a623",

            // Red
            "#e74c3c",

            // Purple
            "#b18cff"
        ]

        Layout.fillWidth: true
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: row.label
                color: Theme.textPrimary
                font.pixelSize: 14
                font.bold: true
                Layout.fillWidth: true
            }

            Rectangle {
                Layout.preferredWidth: 28
                Layout.preferredHeight: 28
                radius: 6
                color: row.value
                border.width: 1
                border.color: Theme.outlineColor
            }

            TextField {
                id: hex
                Layout.preferredWidth: 96
                Layout.preferredHeight: 30
                text: row.value.toString().toUpperCase()
                color: Theme.textPrimary
                font.family: "monospace"
                font.pixelSize: 13
                selectByMouse: true
                maximumLength: 7
                verticalAlignment: TextInput.AlignVCenter
                validator: RegularExpressionValidator { regularExpression: /^#[0-9a-fA-F]{0,6}$/ }

                onEditingFinished: {
                    if (/^#[0-9a-fA-F]{6}$/.test(text))
                        row.picked(text)
                    else
                        text = Qt.binding(() => row.value.toString().toUpperCase())
                }

                background: Rectangle {
                    radius: 6
                    color: hex.activeFocus ? Theme.hoverColor : Theme.inputColor
                    border.width: 1
                    border.color: hex.activeFocus ? Theme.accentColor : Theme.outlineColor
                }
            }
        }

        Flow {
            Layout.fillWidth: true
            spacing: 6

            Repeater {
                model: row.swatches
                Rectangle {
                    required property string modelData
                    width: 22; height: 22; radius: 11
                    color: modelData
                    border.width: Qt.colorEqual(modelData, row.value) ? 2 : 1
                    border.color: Qt.colorEqual(modelData, row.value)
                                  ? Theme.accentColor : Theme.outlineColor
                    scale: swMouse.containsMouse ? 1.15 : 1.0
                    Behavior on scale { NumberAnimation { duration: 100 } }

                    MouseArea {
                        id: swMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: row.picked(parent.modelData)
                    }
                }
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: 0

        // ---------- header ----------
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 16
            Layout.bottomMargin: 8

            Label {
                text: "Settings"
                color: Theme.textPrimary
                font.pixelSize: 18
                font.bold: true
                Layout.fillWidth: true
            }
            MenuButton {
                iconName: "close"
                iconColor: Theme.iconColor
                onClicked: root.close()
            }
        }

        // ---------- tabs ----------
        TabBar {
            id: tabs
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            background: Rectangle {
                color: "transparent"
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width; height: 1
                    color: Theme.borderColor
                }
            }

            // add more tabs here later (General, Network, ...) + a page in the StackLayout
            TabButton {
                id: uiTab
                text: "UI"
                width: implicitWidth + 24
                contentItem: Text {
                    text: uiTab.text
                    font.pixelSize: 14
                    font.bold: uiTab.checked
                    color: uiTab.checked ? Theme.textPrimary : Theme.textMuted
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: "transparent"
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width; height: 2
                        color: uiTab.checked ? Theme.accentColor : "transparent"
                    }
                }
            }
        }

        // ---------- pages ----------
        StackLayout {
            currentIndex: tabs.currentIndex
            Layout.fillWidth: true

            // ===== UI tab =====
            ColumnLayout {
                spacing: 18

                // presets
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: 16
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    spacing: 8

                    Label {
                        text: "Presets"
                        color: Theme.textPrimary
                        font.pixelSize: 14
                        font.bold: true
                    }

                    Row {
                        spacing: 8
                        Repeater {
                            model: Theme.presets
                            Rectangle {
                                id: chip
                                required property var modelData
                                width: chipText.implicitWidth + 28
                                height: 30
                                radius: 15
                                color: chipMouse.containsMouse ? Theme.hoverColor : Theme.inputColor
                                border.width: 1
                                border.color: Theme.outlineColor

                                Row {
                                    anchors.centerIn: parent
                                    spacing: 6
                                    Rectangle {
                                        width: 10; height: 10; radius: 5
                                        anchors.verticalCenter: parent.verticalCenter
                                        color: chip.modelData.accent
                                    }
                                    Text {
                                        id: chipText
                                        text: chip.modelData.name
                                        color: Theme.textPrimary
                                        font.pixelSize: 13
                                    }
                                }
                                MouseArea {
                                    id: chipMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: Theme.applyPreset(chip.modelData)
                                }
                            }
                        }
                    }
                }

                // map style
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    spacing: 8

                    Label {
                        text: "Map style"
                        color: Theme.textPrimary
                        font.pixelSize: 14
                        font.bold: true
                    }

                    Row {
                        spacing: 8
                        Repeater {
                            model: [
                                { name: "Light",       value: "light" },
                                { name: "Dark",        value: "dark"  },
                                { name: "Match theme", value: "theme" }
                            ]
                            Rectangle {
                                id: mapChip
                                required property var modelData
                                readonly property bool selected: Theme.mapStyle === modelData.value
                                width: mapChipText.implicitWidth + 28
                                height: 30
                                radius: 15
                                color: mapChipMouse.containsMouse ? Theme.hoverColor : Theme.inputColor
                                border.width: selected ? 2 : 1
                                border.color: selected ? Theme.accentColor : Theme.outlineColor

                                Text {
                                    id: mapChipText
                                    anchors.centerIn: parent
                                    text: mapChip.modelData.name
                                    color: Theme.textPrimary
                                    font.pixelSize: 13
                                }
                                MouseArea {
                                    id: mapChipMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: Theme.mapStyle = mapChip.modelData.value
                                }
                            }
                        }
                    }
                }

                // color editors
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    spacing: 18

                    ColorRow {
                        label: "Background color"
                        value: Theme.backgroundColor
                        onPicked: (c) => Theme.backgroundColor = c
                    }
                    ColorRow {
                        label: "Accent color"
                        value: Theme.accentColor
                        onPicked: (c) => Theme.accentColor = c
                    }
                    ColorRow {
                        label: "Icons color"
                        value: Theme.iconColor
                        onPicked: (c) => Theme.iconColor = c
                    }
                }

                // footer
                RowLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    Layout.bottomMargin: 16

                    Item { Layout.fillWidth: true }

                    Button {
                        id: resetBtn
                        text: "Reset to default"
                        onClicked: Theme.resetAll()
                        contentItem: Text {
                            text: resetBtn.text
                            color: Theme.textPrimary
                            font.pixelSize: 13
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            radius: 8
                            implicitHeight: 32
                            implicitWidth: 130
                            color: resetBtn.hovered ? Theme.hoverColor : Theme.inputColor
                            border.width: 1
                            border.color: Theme.outlineColor
                        }
                    }
                }
            }
        }
    }
}
