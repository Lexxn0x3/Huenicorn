import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Screen preview with draggable per-channel UV zones and gamma slider
ColumnLayout {
    id: screenArea
    spacing: 8

    property real aspect: huenicorn.displayWidth > 0 && huenicorn.displayHeight > 0
        ? huenicorn.displayWidth / huenicorn.displayHeight
        : 16 / 9

    property var selectedChannel: {
        huenicorn.channelsVersion; // re-evaluate when channels are reloaded
        if (huenicorn.selectedChannelId < 0) {
            return {};
        }
        return huenicorn.channelData(huenicorn.selectedChannelId);
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

        Rectangle {
            id: screen
            anchors.centerIn: parent
            width: Math.min(parent.width, parent.height * screenArea.aspect)
            height: width / screenArea.aspect
            color: "#05070a"
            border.color: Theme.border
            border.width: 1
            radius: Theme.radiusSmall

            // grid
            Repeater {
                model: 7
                delegate: Rectangle {
                    x: (index + 1) / 8 * screen.width
                    width: 1
                    height: screen.height
                    color: Theme.border
                    opacity: 0.6
                }
            }
            Repeater {
                model: 7
                delegate: Rectangle {
                    y: (index + 1) / 8 * screen.height
                    width: screen.width
                    height: 1
                    color: Theme.border
                    opacity: 0.6
                }
            }

            // channel zones
            Repeater {
                model: huenicorn.channels
                delegate: Rectangle {
                    id: zone
                    property int channelId: model.channelId
                    property bool selected: huenicorn.selectedChannelId === channelId

                    visible: model.active
                    x: model.uvMinX * screen.width
                    y: model.uvMinY * screen.height
                    width: Math.max(0, (model.uvMaxX - model.uvMinX) * screen.width)
                    height: Math.max(0, (model.uvMaxY - model.uvMinY) * screen.height)
                    radius: 4
                    color: Theme.channelColor(channelId)
                    opacity: selected ? 0.45 : 0.16
                    border.color: Theme.channelColor(channelId)
                    border.width: selected ? 2 : 1

                    MouseArea {
                        anchors.fill: parent
                        onClicked: huenicorn.selectedChannelId = zone.channelId
                    }

                    // corner handles (only for the selected channel)
                    Repeater {
                        model: [
                            { corner: 0, hx: 0.0, hy: 0.0 },
                            { corner: 1, hx: 1.0, hy: 0.0 },
                            { corner: 2, hx: 0.0, hy: 1.0 },
                            { corner: 3, hx: 1.0, hy: 1.0 }
                        ]
                        delegate: Rectangle {
                            visible: zone.selected
                            width: 14
                            height: 14
                            radius: 7
                            x: modelData.hx * zone.width - width / 2
                            y: modelData.hy * zone.height - height / 2
                            color: Theme.channelColor(zone.channelId)
                            border.color: "#ffffff"
                            border.width: 1

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.SizeAllCursor
                                onClicked: huenicorn.selectedChannelId = zone.channelId
                                onPositionChanged: function(mouse) {
                                    var p = mapToItem(screen, mouse.x, mouse.y);
                                    huenicorn.setChannelUV(zone.channelId, modelData.corner,
                                                           p.x / screen.width, p.y / screen.height);
                                }
                            }
                        }
                    }
                }
            }

            // overlay legend
            Label {
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 8
                color: Theme.textDim
                font.pixelSize: 12
                text: {
                    if (huenicorn.selectedChannelId < 0) {
                        return huenicorn.channels && huenicorn.channels.rowCount() > 0
                            ? "Select a light on the right to edit its screen zone"
                            : "No channel available for the current entertainment area";
                    }
                    var c = screenArea.selectedChannel;
                    var w = Math.round((c.uvMaxX - c.uvMinX) * 100);
                    var h = Math.round((c.uvMaxY - c.uvMinY) * 100);
                    return c.label + "  -  " + w + "% x " + h + "% of the screen";
                }
            }
        }
    }

    // gamma slider for the selected channel
    RowLayout {
        Layout.fillWidth: true
        visible: huenicorn.selectedChannelId >= 0
        spacing: 8

        Label {
            text: "Gamma"
            color: Theme.textDim
        }
        Slider {
            id: gammaSlider
            Layout.fillWidth: true
            from: 0
            to: 1000
            value: {
                var c = screenArea.selectedChannel;
                if (c.gammaFactor === undefined) {
                    return 500;
                }
                return (c.gammaFactor + 1) / 2 * 1000;
            }
            onMoved: {
                var c = screenArea.selectedChannel;
                if (c.channelId !== undefined) {
                    huenicorn.setChannelGammaFactor(c.channelId, value / 1000 * 2 - 1);
                }
            }
        }
        Label {
            color: Theme.text
            font.pixelSize: 12
            text: {
                var c = screenArea.selectedChannel;
                return c.gammaFactor !== undefined ? (Math.round(c.gammaFactor * 100) / 100).toFixed(2) : "";
            }
        }
    }
}
