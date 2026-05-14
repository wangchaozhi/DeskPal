import QtQuick

Window {
    id: petWindow

    width: appController.currentPetWidth > 0 ? appController.currentPetWidth : 220
    height: appController.currentPetHeight > 0 ? appController.currentPetHeight : 250
    visible: true
    color: "transparent"
    opacity: appController.petOpacity
    title: qsTr("DeskPal")
    flags: Qt.FramelessWindowHint
           | Qt.Tool
           | (appController.alwaysOnTop ? Qt.WindowStaysOnTopHint : 0)

    property bool facingLeft: false
    property real walkTargetX: x
    readonly property int edgeSnapMargin: 72

    Component.onCompleted: {
        const position = appController.windowPosition()
        x = position.x
        y = position.y
        clampToScreen()
        appController.setPetVisible(visible)
    }

    onVisibleChanged: {
        appController.setPetVisible(visible)
        if (!visible) {
            stopWalking()
        }
    }

    function clampToScreen() {
        const screen = appController.availableGeometry(x + width / 2, y + height / 2)
        x = Math.max(screen.x, Math.min(x, screen.x + screen.width - width))
        y = Math.max(screen.y, Math.min(y, screen.y + screen.height - height))
    }

    function showSpeech() {
        speechLabel.text = appController.randomSpeech()
        speechBubble.opacity = 1
        speechTimer.restart()
    }

    function walkTo(targetX) {
        if (dragArea.pressed || !petWindow.visible) {
            return
        }

        const screen = appController.availableGeometry(x + width / 2, y + height / 2)
        const minX = screen.x
        const maxX = screen.x + screen.width - width
        let clamped = Math.max(minX, Math.min(targetX, maxX))

        if (clamped - minX < petWindow.edgeSnapMargin) {
            clamped = minX
        } else if (maxX - clamped < petWindow.edgeSnapMargin) {
            clamped = maxX
        }

        if (Math.abs(clamped - petWindow.x) < 2) {
            return
        }

        petWindow.facingLeft = clamped < petWindow.x
        petWindow.walkTargetX = clamped
        appController.setPetWalking(true)
        walkAnimation.restart()
    }

    function startWander() {
        if (dragArea.pressed || walkAnimation.running || !petWindow.visible) {
            return
        }

        const screen = appController.availableGeometry(x + width / 2, y + height / 2)
        const minX = screen.x
        const maxX = screen.x + screen.width - width
        walkTo(minX + Math.random() * Math.max(1, maxX - minX))
    }

    function stopWalking() {
        walkAnimation.stop()
        appController.setPetWalking(false)
    }

    Connections {
        target: appController

        function onShowRequested() {
            petWindow.show()
            petWindow.raise()
            petWindow.requestActivate()
        }

        function onHideRequested() {
            petWindow.hide()
        }

        function onResetPositionRequested() {
            petWindow.stopWalking()
            const position = appController.windowPosition()
            petWindow.x = position.x
            petWindow.y = position.y
            petWindow.clampToScreen()
            appController.saveWindowPosition(petWindow.x, petWindow.y)
        }

        function onAlwaysOnTopChanged() {
            petWindow.flags = Qt.FramelessWindowHint
                    | Qt.Tool
                    | (appController.alwaysOnTop ? Qt.WindowStaysOnTopHint : 0)
            petWindow.show()
        }

        function onCurrentPetChanged() {
            petWindow.clampToScreen()
        }

        function onWanderEnabledChanged() {
            if (!appController.wanderEnabled) {
                petWindow.stopWalking()
            }
        }
    }

    Timer {
        id: wanderTimer
        interval: 7000 + Math.round(Math.random() * 9000)
        repeat: true
        running: appController.wanderEnabled && petWindow.visible && !dragArea.pressed
        onTriggered: {
            interval = 7000 + Math.round(Math.random() * 9000)
            petWindow.startWander()
        }
    }

    NumberAnimation {
        id: walkAnimation
        target: petWindow
        property: "x"
        to: petWindow.walkTargetX
        duration: Math.max(400, Math.abs(petWindow.walkTargetX - petWindow.x) * 14)
        easing.type: Easing.InOutQuad
        onStopped: {
            appController.setPetWalking(false)
            appController.saveWindowPosition(petWindow.x, petWindow.y)
        }
    }

    Loader {
        id: petRenderer
        anchors.centerIn: parent
        scale: appController.currentPetScale > 0 ? appController.currentPetScale : 1.0
        transformOrigin: Item.Center
        sourceComponent: appController.currentPetType === "3d" ? pet3DComponent : pet2DComponent

        transform: Scale {
            origin.x: petRenderer.width / 2
            origin.y: petRenderer.height / 2
            xScale: petWindow.facingLeft ? -1 : 1

            Behavior on xScale { NumberAnimation { duration: 160 } }
        }
    }

    Component {
        id: pet2DComponent

        PetAsset2D {
            pressed: dragArea.pressed
            action: appController.petAction
            petId: appController.currentPetId
            renderer: appController.currentPetRenderer
            source: appController.currentPetSource
        }
    }

    Component {
        id: pet3DComponent

        PetAsset3D {
            pressed: dragArea.pressed
            action: appController.petAction
            petId: appController.currentPetId
            renderer: appController.currentPetRenderer
            source: appController.currentPetSource
        }
    }

    Rectangle {
        id: speechBubble
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 4
        width: Math.min(petWindow.width - 16, speechLabel.implicitWidth + 24)
        height: speechLabel.implicitHeight + 16
        radius: 12
        color: "#ffffff"
        border.color: "#d7dce5"
        opacity: 0
        visible: opacity > 0

        Behavior on opacity { NumberAnimation { duration: 180 } }

        Text {
            id: speechLabel
            anchors.centerIn: parent
            width: petWindow.width - 40
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            color: "#1f2937"
            font.pixelSize: 13
        }

        Rectangle {
            width: 12
            height: 12
            rotation: 45
            color: parent.color
            border.color: parent.border.color
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.bottom
            anchors.topMargin: -6
        }
    }

    Timer {
        id: speechTimer
        interval: 2600
        onTriggered: speechBubble.opacity = 0
    }

    MouseArea {
        id: dragArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        property point dragOffset: Qt.point(0, 0)

        onPressed: mouse => {
            if (mouse.button === Qt.LeftButton) {
                petWindow.stopWalking()
                dragOffset = Qt.point(mouse.x, mouse.y)
                appController.setPetDragging(true)
            } else if (mouse.button === Qt.RightButton) {
                appController.showContextMenu()
            }
        }

        onPositionChanged: mouse => {
            if (pressedButtons & Qt.LeftButton) {
                petWindow.x += mouse.x - dragOffset.x
                petWindow.y += mouse.y - dragOffset.y
                petWindow.clampToScreen()
            }
        }

        onReleased: mouse => {
            if (mouse.button === Qt.LeftButton) {
                appController.setPetDragging(false)
                appController.saveWindowPosition(petWindow.x, petWindow.y)

                const screen = appController.availableGeometry(petWindow.x + petWindow.width / 2,
                                                               petWindow.y + petWindow.height / 2)
                const minX = screen.x
                const maxX = screen.x + screen.width - petWindow.width
                if (petWindow.x - minX < petWindow.edgeSnapMargin) {
                    petWindow.walkTo(minX)
                } else if (maxX - petWindow.x < petWindow.edgeSnapMargin) {
                    petWindow.walkTo(maxX)
                }
            }
        }

        onClicked: mouse => {
            if (mouse.button === Qt.LeftButton) {
                appController.triggerPetAction("happy", 1600)
                petWindow.showSpeech()
            }
        }
    }
}
