// ResourceParameters.qml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import faraz

Rectangle {
    id: root

    readonly property color accent: Theme.accentColor
    readonly property color surface: Theme.inputColor
    readonly property color surfaceHover: Theme.hoverColor
    readonly property color outline: Theme.outlineColor
    readonly property color textIdle: Theme.textMuted

    readonly property var unitFactors: [1, 1e3, 1e6, 1e9]
    readonly property var spans: [50e3, 100e3, 200e3, 500e3, 1e6, 5e6, 10e6]
    readonly property var spanLabels: ["50 kHz", "100 kHz", "200 kHz", "500 kHz", "1 MHz", "5 MHz", "10 MHz"]

    property var stationState: null
    signal tuningRequested(real centerFrequencyHz, real spanHz, bool dfMode)

    color: Theme.panelColor
    radius: 8
    border.width: 1
    border.color: Theme.borderColor

    function currentCenterHz() {
        const value = parseFloat(freqField.text)
        if (isNaN(value))
            return root.stationState ? root.stationState.centerFrequency : 100e6
        return Math.max(0, value * unitFactors[unitBox.currentIndex])
    }

    function currentSpanHz() {
        const idx = Math.round(freqSlider.value)
        return (idx >= 0 && idx < spans.length)
               ? spans[idx]
               : (root.stationState ? root.stationState.span : 1e6)
    }

    function sendCurrentTuning() {
        root.tuningRequested(currentCenterHz(), currentSpanHz(), dfSwitch.checked)
    }

    function syncFromState() {
        if (!stationState)
            return

        const freq = stationState.centerFrequency
        let idx = 2
        for (let i = unitFactors.length - 1; i >= 0; --i) {
            if (freq >= unitFactors[i]) {
                idx = i
                break
            }
        }
        unitBox.currentIndex = idx
        freqField.text = String(freq / unitFactors[idx])

        const s = spans.indexOf(stationState.span)
        freqSlider.value = s >= 0 ? s : 4
        dfSwitch.checked = stationState.dfMode
    }

    Component.onCompleted: syncFromState()
    onStationStateChanged: syncFromState()

    Connections {
        target: root.stationState
        ignoreUnknownSignals: true
        function onCenterFrequencyChanged() { root.syncFromState() }
        function onSpanChanged() { root.syncFromState() }
        function onDfModeChanged() { root.syncFromState() }
    }

// ##########################################################################################
// ##########################################################################################
// ##                                                                                      ##
// ##   !!!!!!!!!!   FAKE-PARAMS  /  PROTOTYPE-ONLY  CODE  --  BEGIN reusable fake widgets  (part 1 of 2)   !!!!!!!!!!   ##
// ##                                                                                      ##
// ##   Nothing in here is connected to the backend. It only exists to make the         ##
// ##   panel look fuller in the prototype.  TO REMOVE: delete BOTH marked blocks          ##
// ##   (2 blocks = 4 banners: BEGIN/END of part 1, BEGIN/END of part 2).                 ##
// ##   Your real controls (Frequency / Span / DF Mode) do not depend on any of it.       ##
// ##                                                                                      ##
// ##########################################################################################
// ##########################################################################################

    component FakeLabel: Label {
        font.pixelSize: 14
        font.bold: true
        color: Theme.textPrimary
    }

    component FakeCombo: ComboBox {
        id: fc
        implicitWidth: 180
        implicitHeight: 32
        font.pixelSize: 14

        contentItem: Text {
            leftPadding: 10
            text: fc.displayText
            font: fc.font
            color: Theme.textPrimary
            verticalAlignment: Text.AlignVCenter
        }
        indicator: Text {
            x: fc.width - width - 10
            y: (fc.height - height) / 2
            text: "\u25BE"
            font.pixelSize: 12
            color: fc.hovered ? Theme.textPrimary : Theme.textMuted
            rotation: fc.popup.visible ? 180 : 0
            Behavior on rotation { NumberAnimation { duration: 150 } }
        }
        background: Rectangle {
            radius: 8
            color: fc.hovered || fc.popup.visible ? Theme.hoverColor : Theme.inputColor
            border.width: 1
            border.color: fc.popup.visible ? Theme.textPrimary
                        : fc.hovered ? Theme.outlineHover
                        : Theme.outlineColor
        }
        delegate: ItemDelegate {
            id: fcItem
            required property var modelData
            required property int index
            width: fc.width - 8
            height: 30
            highlighted: fc.highlightedIndex === index
            contentItem: Text {
                text: fcItem.modelData
                font: fc.font
                color: fcItem.highlighted ? Theme.textPrimary : Theme.textMuted
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                radius: 6
                color: fcItem.highlighted ? Theme.hoverColor : "transparent"
            }
        }
        popup: Popup {
            y: fc.height + 4
            width: fc.width
            padding: 4
            implicitHeight: contentItem.implicitHeight + 8
            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: fc.popup.visible ? fc.delegateModel : null
                currentIndex: fc.highlightedIndex
            }
            background: Rectangle {
                radius: 8
                color: Theme.inputColor
                border.width: 1
                border.color: Theme.outlineColor
            }
        }
    }

    component FakeSlider: Slider {
        id: fs
        property int ticks: 0
        implicitWidth: 180
        implicitHeight: 26

        background: Rectangle {
            x: fs.leftPadding
            y: fs.topPadding + fs.availableHeight / 2 - height / 2
            width: fs.availableWidth
            height: 4
            radius: 2
            color: Theme.grooveColor

            Rectangle {
                width: fs.visualPosition * (parent.width - fs.handle.width) + fs.handle.width / 2
                height: parent.height
                radius: 2
                color: Theme.accentColor
            }
            Repeater {
                model: fs.ticks > 1 ? fs.ticks : 0
                Rectangle {
                    required property int index
                    width: 1
                    height: 9
                    color: Theme.outlineColor
                    x: fs.handle.width / 2 + index / (fs.ticks - 1) * (parent.width - fs.handle.width) - width / 2
                    y: (parent.height - height) / 2
                    z: -1
                }
            }
        }
        handle: Rectangle {
            x: fs.leftPadding + fs.visualPosition * (fs.availableWidth - width)
            y: fs.topPadding + fs.availableHeight / 2 - height / 2
            width: 12
            height: 20
            radius: 6
            color: fs.pressed ? Theme.textSoft : Theme.textPrimary
            scale: fs.hovered || fs.pressed ? 1.1 : 1.0
            Behavior on scale { NumberAnimation { duration: 120 } }
        }
    }

    component FakeSwitch: Switch {
        id: fsw
        indicator: Rectangle {
            implicitWidth: 42
            implicitHeight: 22
            x: fsw.leftPadding
            y: parent.height / 2 - height / 2
            radius: height / 2
            color: fsw.checked ? Theme.accentColor : Theme.inputColor
            border.width: 1
            border.color: fsw.checked ? Theme.accentColor
                        : fsw.hovered ? Theme.outlineHover
                        : Theme.outlineColor
            Behavior on color { ColorAnimation { duration: 150 } }

            Rectangle {
                width: 16
                height: 16
                radius: 8
                color: Theme.textPrimary
                y: (parent.height - height) / 2
                x: 3 + fsw.visualPosition * (parent.width - width - 6)
                Behavior on x { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }
            }
        }
        contentItem: Text {
            text: fsw.text
            font.pixelSize: 14
            font.bold: fsw.checked
            color: fsw.checked ? Theme.textPrimary : Theme.textMuted
            verticalAlignment: Text.AlignVCenter
            leftPadding: fsw.indicator.width + fsw.spacing
        }
    }

    // number box with a unit label on the right, e.g.  [   -97 ] dBm
    component FakeValueField: RowLayout {
        id: vf
        property alias text: tf.text
        property alias readOnly: tf.readOnly
        property string unit: ""
        spacing: 8

        TextField {
            id: tf
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            color: Theme.textPrimary
            font.pixelSize: 14
            selectByMouse: true
            horizontalAlignment: TextInput.AlignRight
            verticalAlignment: TextInput.AlignVCenter
            rightPadding: 10
            background: Rectangle {
                radius: 8
                color: tf.hovered || tf.activeFocus ? Theme.hoverColor : Theme.inputColor
                border.width: 1
                border.color: tf.activeFocus ? Theme.textPrimary
                            : tf.hovered ? Theme.outlineHover
                            : Theme.outlineColor
            }
        }
        Text {
            visible: vf.unit.length > 0
            text: vf.unit
            color: Theme.textMuted
            font.pixelSize: 13
        }
    }

    // collapsible accordion section (header + body)
    component FakeSection: ColumnLayout {
        id: sec
        property string title
        property bool expanded: false
        default property alias content: body.data
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            radius: 6
            color: secMouse.containsMouse ? Theme.hoverColor : "transparent"
            border.width: sec.expanded ? 1 : 0
            border.color: Theme.accentColor

            Row {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 8
                spacing: 8
                Text {
                    text: "\u25BE"
                    font.pixelSize: 12
                    color: Theme.textPrimary
                    anchors.verticalCenter: parent.verticalCenter
                    rotation: sec.expanded ? 180 : 0
                    Behavior on rotation { NumberAnimation { duration: 150 } }
                }
                Text {
                    text: sec.title
                    font.pixelSize: 13
                    font.bold: true
                    color: Theme.textPrimary
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            MouseArea {
                id: secMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: sec.expanded = !sec.expanded
            }
        }

        ColumnLayout {
            id: body
            visible: sec.expanded
            Layout.fillWidth: true
            Layout.topMargin: 8
            Layout.bottomMargin: 4
            spacing: 10
        }
    }

// ##########################################################################################
// ##########################################################################################
// ##                                                                                      ##
// ##   !!!!!!!!!!   FAKE-PARAMS  /  PROTOTYPE-ONLY  CODE  --  END reusable fake widgets  (part 1 of 2)   !!!!!!!!!!   ##
// ##                                                                                      ##
// ##   Nothing in here is connected to the backend. It only exists to make the         ##
// ##   panel look fuller in the prototype.  TO REMOVE: delete BOTH marked blocks          ##
// ##   (2 blocks = 4 banners: BEGIN/END of part 1, BEGIN/END of part 2).                 ##
// ##   Your real controls (Frequency / Span / DF Mode) do not depend on any of it.       ##
// ##                                                                                      ##
// ##########################################################################################
// ##########################################################################################

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
                color: Theme.textPrimary
            }

            Row {
                spacing: 6

                TextField {
                    id: freqField
                    width: 120
                    height: 32
                    leftPadding: 10
                    rightPadding: 10
                    color: Theme.textPrimary
                    font.pixelSize: 14
                    placeholderText: "Value"
                    placeholderTextColor: Theme.textDim
                    selectByMouse: true
                    verticalAlignment: TextInput.AlignVCenter
                    validator: DoubleValidator { bottom: 0 }

                    onEditingFinished: root.sendCurrentTuning()

                    background: Rectangle {
                        radius: 8
                        color: freqField.hovered || freqField.activeFocus ? root.surfaceHover : root.surface
                        border.width: 1
                        border.color: freqField.activeFocus ? Theme.textPrimary
                                    : freqField.hovered ? Theme.outlineHover
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

                    onActivated: root.sendCurrentTuning()

                    contentItem: Text {
                        leftPadding: 10
                        text: unitBox.displayText
                        font: unitBox.font
                        color: Theme.textPrimary
                        verticalAlignment: Text.AlignVCenter
                    }

                    indicator: Text {
                        x: unitBox.width - width - 10
                        y: (unitBox.height - height) / 2
                        text: "\u25BE"
                        font.pixelSize: 12
                        color: unitBox.hovered ? Theme.textPrimary : root.textIdle
                        rotation: unitBox.popup.visible ? 180 : 0
                        Behavior on rotation { NumberAnimation { duration: 150 } }
                    }

                    background: Rectangle {
                        radius: 8
                        color: unitBox.hovered || unitBox.popup.visible ? root.surfaceHover : root.surface
                        border.width: 1
                        border.color: unitBox.popup.visible ? Theme.textPrimary
                                    : unitBox.hovered ? Theme.outlineHover
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
                            color: unitItem.highlighted ? Theme.textPrimary : root.textIdle
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            radius: 6
                            color: unitItem.highlighted ? Theme.hoverColor : "transparent"
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
                            border.color: Theme.outlineColor
                        }
                    }
                }
            }

            Label {
                text: "Span:"
                font.pixelSize: 14
                font.bold: true
                color: Theme.textPrimary
            }

            Slider {
                id: freqSlider

                Layout.preferredWidth: 220
                Layout.preferredHeight: 55

                from: 0
                to: 6
                stepSize: 1
                snapMode: Slider.SnapAlways

                onMoved: root.sendCurrentTuning()

                background: Rectangle {
                    x: freqSlider.leftPadding
                    y: freqSlider.topPadding +
                       freqSlider.availableHeight / 2 - height / 2

                    width: freqSlider.availableWidth
                    height: 4
                    radius: 2
                    color: Theme.grooveColor

                    Rectangle {
                        width: freqSlider.visualPosition *
                               (parent.width - freqSlider.handle.width) +
                               freqSlider.handle.width / 2
                        height: parent.height
                        radius: 2
                        color: root.accent
                    }

                    Repeater {
                        model: 7
                        Rectangle {
                            required property int index
                            width: 2
                            height: 10
                            radius: 1
                            color: Theme.outlineColor
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
                    color: freqSlider.pressed ? Theme.textSoft : Theme.textPrimary
                    scale: freqSlider.hovered || freqSlider.pressed ? 1.1 : 1.0
                    Behavior on scale { NumberAnimation { duration: 120 } }
                }

                Text {
                    text: root.spanLabels[Math.round(freqSlider.value)]
                    color: Theme.textPrimary
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

                onToggled: root.sendCurrentTuning()

                indicator: Rectangle {
                    implicitWidth: 42
                    implicitHeight: 22
                    x: dfSwitch.leftPadding
                    y: parent.height / 2 - height / 2
                    radius: height / 2
                    color: dfSwitch.checked ? root.accent : root.surface
                    border.width: 1
                    border.color: dfSwitch.checked ? root.accent
                                : dfSwitch.hovered ? Theme.outlineHover
                                : root.outline
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Behavior on border.color { ColorAnimation { duration: 150 } }

                    Rectangle {
                        width: 16
                        height: 16
                        radius: 8
                        color: Theme.textPrimary
                        y: (parent.height - height) / 2
                        x: 3 + dfSwitch.visualPosition * (parent.width - width - 6)
                        Behavior on x { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }
                    }
                }

                contentItem: Text {
                    text: dfSwitch.text
                    font.pixelSize: 14
                    font.bold: dfSwitch.checked
                    color: dfSwitch.checked ? Theme.textPrimary : root.textIdle
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: dfSwitch.indicator.width + dfSwitch.spacing
                    Behavior on color { ColorAnimation { duration: 150 } }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }


// ##########################################################################################
// ##########################################################################################
// ##                                                                                      ##
// ##   !!!!!!!!!!   FAKE-PARAMS  /  PROTOTYPE-ONLY  CODE  --  BEGIN fake columns  (part 2 of 2)   !!!!!!!!!!   ##
// ##                                                                                      ##
// ##   Nothing in here is connected to the backend. It only exists to make the         ##
// ##   panel look fuller in the prototype.  TO REMOVE: delete BOTH marked blocks          ##
// ##   (2 blocks = 4 banners: BEGIN/END of part 1, BEGIN/END of part 2).                 ##
// ##   Your real controls (Frequency / Span / DF Mode) do not depend on any of it.       ##
// ##                                                                                      ##
// ##########################################################################################
// ##########################################################################################

        // thin divider between the real controls and the fake ones
        Rectangle {
            Layout.fillHeight: true
            Layout.topMargin: 14
            Layout.bottomMargin: 14
            Layout.leftMargin: 14
            Layout.preferredWidth: 1
            color: Theme.borderColor
        }

        // ---- fake column A : "Audio and demodulation" ----
        ColumnLayout {
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: 14
            Layout.leftMargin: 14
            Layout.preferredWidth: 200
            spacing: 14

            FakeLabel { text: "Demodulator:" }
            FakeCombo {
                Layout.fillWidth: true
                model: ["AM", "FM", "USB", "LSB", "CW"]
                currentIndex: 1
            }

            FakeLabel { text: "Demodulation bandwidth:" }
            FakeSlider {
                Layout.fillWidth: true
                from: 0; to: 5; stepSize: 1; value: 2
                ticks: 6
            }
            FakeCombo {
                Layout.fillWidth: true
                model: ["6,250 kHz", "12,500 kHz", "25,000 kHz", "50,000 kHz", "100,000 kHz", "200,000 kHz"]
                currentIndex: 2
            }

            FakeSwitch { text: "Squelch"; checked: true }

            Item { Layout.fillHeight: true }
        }

        // ---- fake column B : squelch / gain + accordions ----
        ColumnLayout {
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: 14
            Layout.leftMargin: 14
            Layout.preferredWidth: 200
            spacing: 12

            FakeLabel { text: "Squelch threshold:" }
            FakeSlider {
                id: fakeThreshold
                Layout.fillWidth: true
                from: -130; to: -30; stepSize: 1; value: -97
                onValueChanged: fakeThresholdField.text = String(Math.round(value))
            }
            FakeValueField {
                id: fakeThresholdField
                Layout.fillWidth: true
                text: "-97"
                unit: "dBm"
            }

            FakeLabel { text: "Gain value:" }
            FakeValueField {
                Layout.fillWidth: true
                text: "AUTO"
                unit: "dBm"
                readOnly: true
            }

            Item { Layout.fillHeight: true }
        }

// ##########################################################################################
// ##########################################################################################
// ##                                                                                      ##
// ##   !!!!!!!!!!   FAKE-PARAMS  /  PROTOTYPE-ONLY  CODE  --  END fake columns  (part 2 of 2)   !!!!!!!!!!   ##
// ##                                                                                      ##
// ##   Nothing in here is connected to the backend. It only exists to make the         ##
// ##   panel look fuller in the prototype.  TO REMOVE: delete BOTH marked blocks          ##
// ##   (2 blocks = 4 banners: BEGIN/END of part 1, BEGIN/END of part 2).                 ##
// ##   Your real controls (Frequency / Span / DF Mode) do not depend on any of it.       ##
// ##                                                                                      ##
// ##########################################################################################
// ##########################################################################################
        Item {
            Layout.fillWidth: true
        }
    }
}
