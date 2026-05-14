import QtQuick

Item {
    id: root

    property bool pressed: false
    property string action: "idle"

    width: 180
    height: 210

    onActionChanged: if (action !== "walking") rotation = 0

    transform: Scale {
        origin.x: root.width / 2
        origin.y: root.height
        xScale: root.action === "happy" ? 1.06 : root.pressed ? 0.96 : 1.0
        yScale: root.action === "sleepy" ? 0.94 : root.pressed ? 1.03 : 1.0

        Behavior on xScale { NumberAnimation { duration: 120 } }
        Behavior on yScale { NumberAnimation { duration: 120 } }
    }

    SequentialAnimation on y {
        loops: Animation.Infinite
        NumberAnimation { to: root.action === "sleepy" ? -2 : root.action === "happy" ? -10 : root.action === "walking" ? -8 : -5; duration: root.action === "happy" || root.action === "walking" ? 420 : 1200; easing.type: Easing.InOutSine }
        NumberAnimation { to: 0; duration: root.action === "happy" || root.action === "walking" ? 420 : 1200; easing.type: Easing.InOutSine }
    }

    SequentialAnimation on rotation {
        running: root.action === "walking"
        loops: Animation.Infinite
        NumberAnimation { to: 5; duration: 260; easing.type: Easing.InOutSine }
        NumberAnimation { to: -5; duration: 260; easing.type: Easing.InOutSine }
    }

    Rectangle {
        id: shadow
        width: 126
        height: 20
        radius: 10
        color: "#33000000"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 4
    }

    Rectangle {
        id: body
        width: 136
        height: 154
        radius: 58
        color: root.action === "sleepy" ? "#d8d3eb" : root.action === "happy" ? "#ffd966" : root.action === "dragging" ? "#f2b84b" : "#f7cf5e"
        border.width: 3
        border.color: "#3a2c1a"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: shadow.top
        anchors.bottomMargin: -2
    }

    Rectangle {
        width: 48
        height: 48
        radius: 24
        color: "#f2b84b"
        border.width: 3
        border.color: "#3a2c1a"
        anchors.left: body.left
        anchors.top: body.top
        anchors.leftMargin: -10
        anchors.topMargin: 42
    }

    Rectangle {
        width: 48
        height: 48
        radius: 24
        color: "#f2b84b"
        border.width: 3
        border.color: "#3a2c1a"
        anchors.right: body.right
        anchors.top: body.top
        anchors.rightMargin: -10
        anchors.topMargin: 42
    }

    Rectangle {
        width: 26
        height: root.action === "sleepy" ? 8 : 34
        radius: 13
        color: "#2f2519"
        anchors.left: body.left
        anchors.top: body.top
        anchors.leftMargin: 36
        anchors.topMargin: 48

        Rectangle {
            width: 8
            height: 10
            radius: 4
            color: "#ffffff"
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 6
            anchors.topMargin: 6
        }
    }

    Rectangle {
        width: 26
        height: root.action === "sleepy" ? 8 : 34
        radius: 13
        color: "#2f2519"
        anchors.right: body.right
        anchors.top: body.top
        anchors.rightMargin: 36
        anchors.topMargin: 48

        Rectangle {
            width: 8
            height: 10
            radius: 4
            color: "#ffffff"
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 6
            anchors.topMargin: 6
        }
    }

    Rectangle {
        width: root.action === "happy" ? 46 : 34
        height: root.action === "happy" ? 24 : 18
        radius: height / 2
        color: "#df7f6c"
        anchors.horizontalCenter: body.horizontalCenter
        anchors.top: body.top
        anchors.topMargin: root.pressed ? 92 : 88
    }

    Rectangle {
        width: 62
        height: 30
        radius: 15
        color: "#ffe38b"
        anchors.horizontalCenter: body.horizontalCenter
        anchors.bottom: body.bottom
        anchors.bottomMargin: 20
    }
}
