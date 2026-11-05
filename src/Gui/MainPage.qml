import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: mainPage

    property bool savedRecently: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacing
        spacing: Theme.spacing

        // header
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Image {
                source: "qrc:/qt/qml/huenicorn_UI/logo.svg"
                sourceSize.width: 30
                sourceSize.height: 30
                fillMode: Image.PreserveAspectFit
            }
            Label {
                text: "Huenicorn"
                font.pixelSize: 18
                font.bold: true
                color: Theme.text
            }
            Label {
                text: "v" + huenicorn.version
                color: Theme.textDim
                font.pixelSize: 12
            }
            Item { Layout.fillWidth: true }
            Button {
                text: "Shut down"
                onClicked: confirmStopDialog.open()
                background: Rectangle {
                    color: parent.hovered ? Theme.danger : Theme.surfaceActive
                    border.color: Theme.border
                    border.width: 1
                    radius: Theme.radiusSmall
                }
                contentItem: Label {
                    text: parent.text
                    color: parent.hovered ? "#ffffff" : Theme.text
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            visible: !huenicorn.running
            color: Theme.accentSoft
            border.color: Theme.accent
            border.width: 1
            radius: Theme.radiusSmall
            implicitHeight: bannerLabel.implicitHeight + 16

            Label {
                id: bannerLabel
                anchors.centerIn: parent
                width: parent.width - 24
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                color: Theme.text
                text: "Waiting for screen capture — pick your screen in the portal dialog and click \"Share\"."
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.spacing

            // left: screen preview card
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Theme.surface
                border.color: Theme.border
                border.width: 1
                radius: Theme.radius

                ScreenArea {
                    anchors.fill: parent
                    anchors.margins: Theme.padding
                }
            }

            // right sidebar
            ColumnLayout {
                Layout.preferredWidth: 350
                Layout.fillHeight: true
                spacing: Theme.spacing

                Label {
                    text: "Entertainment area"
                    color: Theme.textDim
                    font.pixelSize: 12
                    font.bold: true
                }
                ComboBox {
                    id: entertainmentCombo
                    Layout.fillWidth: true
                    model: huenicorn.entertainmentConfigurations
                    textRole: "name"
                    valueRole: "id"
                    currentIndex: indexOfValue(huenicorn.currentEntertainmentConfigurationId)
                    onActivated: huenicorn.selectEntertainmentConfiguration(currentValue)
                    background: Rectangle {
                        color: Theme.background
                        border.color: Theme.border
                        border.width: 1
                        radius: Theme.radiusSmall
                    }
                    contentItem: Label {
                        text: parent.displayText
                        color: Theme.text
                        leftPadding: 10
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                }

                Label {
                    text: "Lights"
                    color: Theme.textDim
                    font.pixelSize: 12
                    font.bold: true
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: Theme.background
                    border.color: Theme.border
                    border.width: 1
                    radius: Theme.radiusSmall

                    ListView {
                        id: channelsList
                        anchors.fill: parent
                        anchors.margins: 4
                        clip: true
                        model: huenicorn.channels
                        spacing: 4

                        delegate: Rectangle {
                            width: channelsList.width
                            height: 52
                            radius: Theme.radiusSmall
                            color: huenicorn.selectedChannelId === model.channelId ? Theme.surfaceActive : (hoverArea.containsMouse ? Theme.surfaceHover : "transparent")
                            border.color: huenicorn.selectedChannelId === model.channelId ? Theme.accent : "transparent"
                            border.width: 1

                            MouseArea {
                                id: hoverArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: huenicorn.selectedChannelId = model.channelId
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 8

                                Rectangle {
                                    width: 14
                                    height: 14
                                    radius: 7
                                    color: Theme.channelColor(model.channelId)
                                    opacity: model.active ? 1.0 : 0.25
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    Label {
                                        text: model.label
                                        color: Theme.text
                                        font.pixelSize: 13
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }
                                    Label {
                                        text: model.devices.length > 0 ? model.devices.join(", ") : "no light"
                                        color: Theme.textDim
                                        font.pixelSize: 11
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }
                                }
                                Switch {
                                    checked: model.active
                                    onToggled: huenicorn.setChannelActivity(model.channelId, checked)
                                }
                            }
                        }
                    }
                }

                // advanced settings
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Label {
                        text: "Advanced settings"
                        color: Theme.textDim
                        font.pixelSize: 12
                        font.bold: true
                        Layout.fillWidth: true
                    }
                    Switch {
                        id: advancedSwitch
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    visible: advancedSwitch.checked

                    Label { text: "Subsample resolution"; color: Theme.textDim; font.pixelSize: 12 }
                    ComboBox {
                        id: subsampleCombo
                        Layout.fillWidth: true
                        model: huenicorn.subsampleCandidates
                        onActivated: huenicorn.setSubsampleWidth(model[index].x)
                        Connections {
                            target: huenicorn
                            function onDisplayInfoChanged() {
                                for (var i = 0; i < subsampleCombo.count; i++) {
                                    if (subsampleCombo.model[i].x === huenicorn.subsampleWidth) {
                                        subsampleCombo.currentIndex = i;
                                        break;
                                    }
                                }
                            }
                        }
                        delegate: ItemDelegate {
                            width: subsampleCombo.width
                            contentItem: Label {
                                text: modelData.x + " x " + modelData.y
                                    + "  (" + Math.round(modelData.x / Math.max(1, huenicorn.displayWidth) * 100) + "%)"
                                color: Theme.text
                            }
                        }
                        background: Rectangle {
                            color: Theme.background
                            border.color: Theme.border
                            border.width: 1
                            radius: Theme.radiusSmall
                        }
                        contentItem: Label {
                            text: subsampleCombo.currentIndex >= 0 && subsampleCombo.model.length > 0
                                ? subsampleCombo.model[subsampleCombo.currentIndex].x + " x " + subsampleCombo.model[subsampleCombo.currentIndex].y
                                : ""
                            color: Theme.text
                            leftPadding: 10
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    Label { text: "Interpolation"; color: Theme.textDim; font.pixelSize: 12 }
                    ComboBox {
                        Layout.fillWidth: true
                        model: huenicorn.interpolations
                        textRole: "name"
                        valueRole: "value"
                        currentIndex: model ? model.findIndex(function(m) { return m.value === huenicorn.interpolation; }) : -1
                        onActivated: huenicorn.setInterpolation(currentValue)
                        background: Rectangle {
                            color: Theme.background
                            border.color: Theme.border
                            border.width: 1
                            radius: Theme.radiusSmall
                        }
                        contentItem: Label {
                            text: parent.displayText
                            color: Theme.text
                            leftPadding: 10
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    Label { text: "Refresh rate (Hz)"; color: Theme.textDim; font.pixelSize: 12 }
                    SpinBox {
                        from: 1
                        to: Math.max(1, huenicorn.maxRefreshRate)
                        value: huenicorn.refreshRate
                        onValueModified: huenicorn.setRefreshRate(value)
                        background: Rectangle {
                            color: Theme.background
                            border.color: Theme.border
                            border.width: 1
                            radius: Theme.radiusSmall
                        }
                        contentItem: TextInput {
                            text: parent.textFromValue(parent.value, parent.locale)
                            color: Theme.text
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            readOnly: !parent.editable
                        }
                    }
                }

                Item { Layout.fillHeight: true }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Button {
                        text: "Save profile"
                        onClicked: {
                            huenicorn.saveProfile();
                            mainPage.savedRecently = true;
                            savedTimer.restart();
                        }
                        background: Rectangle {
                            color: parent.enabled ? Theme.accentSoft : Theme.surfaceActive
                            border.color: Theme.border
                            border.width: 1
                            radius: Theme.radiusSmall
                        }
                        contentItem: Label {
                            text: parent.text
                            color: Theme.text
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                    Label {
                        visible: mainPage.savedRecently
                        text: "Saved"
                        color: Theme.success
                        font.pixelSize: 12
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: huenicorn.displayWidth + "x" + huenicorn.displayHeight
                            + "  -  " + huenicorn.refreshRate + " Hz"
                        color: Theme.textDim
                        font.pixelSize: 11
                    }
                }
            }
        }
    }

    Timer {
        id: savedTimer
        interval: 2500
        onTriggered: mainPage.savedRecently = false
    }

    Dialog {
        id: confirmStopDialog
        anchors.centerIn: Overlay.overlay
        modal: true
        title: "Shut down Huenicorn?"
        standardButtons: Dialog.Yes | Dialog.No
        onAccepted: huenicorn.quit()

        Label {
            text: "Do you really want to stop Huenicorn?"
            color: Theme.text
        }
    }
}
