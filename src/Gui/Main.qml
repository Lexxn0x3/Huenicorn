import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 1180
    height: 720
    minimumWidth: 900
    minimumHeight: 560
    visible: true
    title: "Huenicorn"
    color: Theme.background

    Loader {
        id: pageLoader
        anchors.fill: parent
        sourceComponent: huenicorn.setupRequired ? setupPageComponent : mainPageComponent
    }

    Component {
        id: setupPageComponent
        SetupPage {}
    }

    Component {
        id: mainPageComponent
        MainPage {}
    }
}
