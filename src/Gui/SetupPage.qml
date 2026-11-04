import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Initial setup wizard: bridge discovery / address validation, then user pairing
Item {
    id: setupPage

    property int step: 1
    property bool busy: false
    property string errorMessage: ""
    property var detectedBridges: []

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(520, parent.width - 48)
        height: contentColumn.implicitHeight + 2 * Theme.padding
        color: Theme.surface
        border.color: Theme.border
        border.width: 1
        radius: Theme.radius

        ColumnLayout {
            id: contentColumn
            anchors.fill: parent
            anchors.margins: Theme.padding
            spacing: Theme.spacing

            Image {
                Layout.alignment: Qt.AlignHCenter
                source: "qrc:/qt/qml/huenicorn_UI/logo.svg"
                sourceSize.width: 72
                sourceSize.height: 72
                fillMode: Image.PreserveAspectFit
            }

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: "Huenicorn setup"
                font.pixelSize: 22
                font.bold: true
                color: Theme.text
            }

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                color: Theme.textDim
                text: step === 1
                    ? "Connect Huenicorn to your Philips Hue bridge."
                    : "Press the link button on your Hue bridge, then click Pair."
            }

            // Step 1: bridge address
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8
                visible: step === 1

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    TextField {
                        id: bridgeAddressField
                        Layout.fillWidth: true
                        placeholderText: "Bridge IP address (e.g. 192.168.1.10)"
                        color: Theme.text
                        selectedTextColor: Theme.background
                        selectionColor: Theme.accent
                        background: Rectangle {
                            color: Theme.background
                            border.color: Theme.border
                            border.width: 1
                            radius: Theme.radiusSmall
                        }
                    }

                    Button {
                        text: "Auto-detect"
                        enabled: !setupPage.busy
                        onClicked: {
                            setupPage.busy = true;
                            setupPage.errorMessage = "";
                            var result = huenicorn.autodetectBridges();
                            setupPage.busy = false;
                            if (result.succeeded && result.bridges.length > 0) {
                                setupPage.detectedBridges = result.bridges;
                                bridgeAddressField.text = result.bridges[0].internalipaddress;
                            } else {
                                setupPage.errorMessage = result.error !== undefined
                                    ? result.error
                                    : "No bridge detected. Enter the IP address manually.";
                            }
                        }
                        background: Rectangle {
                            color: parent.enabled ? Theme.surfaceActive : Theme.surface
                            border.color: Theme.border
                            border.width: 1
                            radius: Theme.radiusSmall
                        }
                        contentItem: Label {
                            text: parent.text
                            color: parent.enabled ? Theme.text : Theme.textDim
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                Button {
                    Layout.fillWidth: true
                    text: "Continue"
                    enabled: !setupPage.busy && bridgeAddressField.text.trim() !== ""
                    onClicked: {
                        setupPage.busy = true;
                        setupPage.errorMessage = "";
                        var ok = huenicorn.validateBridgeAddress(bridgeAddressField.text);
                        setupPage.busy = false;
                        if (ok) {
                            setupPage.step = 2;
                        } else {
                            setupPage.errorMessage = "Could not reach a Hue bridge at this address.";
                        }
                    }
                    background: Rectangle {
                        color: parent.enabled ? Theme.accent : Theme.surfaceActive
                        radius: Theme.radiusSmall
                    }
                    contentItem: Label {
                        text: parent.text
                        color: parent.enabled ? "#0b0d10" : Theme.textDim
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            // Step 2: pairing
            Button {
                Layout.fillWidth: true
                visible: step === 2
                text: "Pair with bridge"
                enabled: !setupPage.busy
                onClicked: {
                    setupPage.busy = true;
                    setupPage.errorMessage = "";
                    var result = huenicorn.registerNewUser();
                    setupPage.busy = false;
                    if (!result.succeeded) {
                        setupPage.errorMessage = result.error !== undefined
                            ? result.error
                            : "Pairing failed. Make sure the bridge link button was pressed.";
                    }
                    // On success the controller switches setupRequired off and
                    // Main.qml automatically loads the main page.
                }
                background: Rectangle {
                    color: parent.enabled ? Theme.accent : Theme.surfaceActive
                    radius: Theme.radiusSmall
                }
                contentItem: Label {
                    text: parent.text
                    color: parent.enabled ? "#0b0d10" : Theme.textDim
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            RowLayout {
                Layout.fillWidth: true
                visible: setupPage.busy
                spacing: 8

                BusyIndicator {
                    Layout.alignment: Qt.AlignVCenter
                    running: setupPage.busy
                    palette.dark: Theme.accent
                }
                Label {
                    text: "Contacting bridge..."
                    color: Theme.textDim
                }
            }

            Label {
                Layout.fillWidth: true
                visible: setupPage.errorMessage !== ""
                wrapMode: Text.WordWrap
                color: Theme.danger
                text: setupPage.errorMessage
            }

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                color: Theme.textDim
                font.pixelSize: 11
                text: "Huenicorn " + huenicorn.version + "  -  config: " + huenicorn.configFilePath
            }
        }
    }
}
