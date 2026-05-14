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

    Component.onCompleted: {
        const position = appController.windowPosition()
        x = position.x
        y = position.y
        clampToScreen()
        appController.setPetVisible(visible)
    }

    onVisibleChanged: appController.setPetVisible(visible)

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
    }

    Loader {
        id: petRenderer
        anchors.centerIn: parent
        scale: appController.currentPetScale > 0 ? appController.currentPetScale : 1.0
        transformOrigin: Item.Center
        sourceComponent: appController.currentPetType === "3d" || appController.renderMode === "3d" ? pet3DComponent : pet2DComponent
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
                dragOffset = Qt.point(mouse.x, mouse.y)
                appController.setPetDragging(true)
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
            }
        }

        onClicked: mouse => {
            if (mouse.button === Qt.RightButton) {
                appController.showContextMenu()
            } else if (mouse.button === Qt.LeftButton) {
                appController.triggerPetAction("happy", 1600)
                petWindow.showSpeech()
            }
        }
    }
}
