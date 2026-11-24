import QtQuick
import QtQuick.Layouts

Rectangle {
    id: leftMenuContainer
    Layout.preferredWidth: 50
    Layout.fillHeight: true
    Layout.leftMargin: 5
    // radius: 10
    color: "transparent"

    signal settingsRequested()

    ColumnLayout {
        id: leftMenu
        anchors.fill: parent
        spacing: 40

        MenuButton {
            Layout.topMargin: 12
            iconName: "grid_view"
            onClicked: console.log("clicked home_app_logo")
        }

        MenuButton {
            iconName: "cell_tower"
            onClicked: console.log("clicked search")
        }

        MenuButton {
            iconName: "vital_signs"
            onClicked: console.log("clicked apps")
        }

        MenuButton {
            iconName: "bar_chart"
            onClicked: console.log("clicked apps")
        }

        Item {
            Layout.fillHeight: true
        }

        MenuButton {
            iconName: "security"
            onClicked: console.log("clicked apps")
        }

        MenuButton {
            iconName: "settings"
            onClicked: leftMenuContainer.settingsRequested()
        }

        Image {
            id: name
            Layout.preferredWidth: 20
            Layout.preferredHeight: 20
            source: "qrc:/qt/qml/faraz/resources/images/farazlogo.png"
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 20
        }
    }
}
