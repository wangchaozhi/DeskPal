import QtQuick

Window {
    id: petWindow

    width: 220
    height: 250
    visible: true
    color: "transparent"
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
            petWindow.x = 120
            petWindow.y = 120
            petWindow.clampToScreen()
            appController.saveWindowPosition(petWindow.x, petWindow.y)
        }

        function onAlwaysOnTopChanged() {
            petWindow.flags = Qt.FramelessWindowHint
                    | Qt.Tool
                    | (appController.alwaysOnTop ? Qt.WindowStaysOnTopHint : 0)
            petWindow.show()
        }
    }

    Loader {
        id: petRenderer
        anchors.centerIn: parent
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
            }
        }
    }
}
