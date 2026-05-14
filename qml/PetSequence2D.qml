import QtQuick

Item {
    id: root

    property string petId: appController.currentPetId
    property string action: "idle"
    property var frames: appController.petFrameUrls(petId, action)
    property int frameIndex: 0

    onActionChanged: {
        frames = appController.petFrameUrls(petId, action)
        frameIndex = 0
    }

    onPetIdChanged: {
        frames = appController.petFrameUrls(petId, action)
        frameIndex = 0
    }

    Connections {
        target: appController

        function onCurrentPetChanged() {
            root.frames = appController.petFrameUrls(root.petId, root.action)
            root.frameIndex = 0
        }
    }

    Image {
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        smooth: true
        source: root.frames.length > 0 ? root.frames[root.frameIndex % root.frames.length] : ""
    }

    Timer {
        interval: 90
        repeat: true
        running: root.frames.length > 1
        onTriggered: root.frameIndex = (root.frameIndex + 1) % root.frames.length
    }

    PetBody {
        anchors.centerIn: parent
        visible: root.frames.length === 0
        action: root.action
    }
}
