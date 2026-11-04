pragma Singleton
import QtQuick

// Central theme definition for the Huenicorn Qt GUI (dark theme)
QtObject {
    readonly property color background: "#0e1014"
    readonly property color surface: "#16191f"
    readonly property color surfaceHover: "#1d212a"
    readonly property color surfaceActive: "#242935"
    readonly property color border: "#2c313d"
    readonly property color text: "#e8eaf0"
    readonly property color textDim: "#8b93a3"
    readonly property color accent: "#4da3ff"
    readonly property color accentSoft: "#264da3"
    readonly property color danger: "#ff5c5c"
    readonly property color success: "#3ddc84"

    readonly property int radius: 10
    readonly property int radiusSmall: 6
    readonly property int spacing: 12
    readonly property int padding: 16

    // Deterministic per-channel color used for zone rectangles
    function channelColor(channelId) {
        var hue = (channelId * 67 + 20) % 360;
        return Qt.hsla(hue / 360.0, 0.75, 0.58, 1.0);
    }
}
