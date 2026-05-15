import QtQuick

Item {
    id: root

    property bool pressed: false
    property string action: "idle"
    property string petId: appController.currentPetId
    property string renderer: "qml"
    property string source: ""
    property int fps: appController.petFps(petId)

    width: 180
    height: 210

    Loader {
        id: assetLoader
        anchors.fill: parent
        sourceComponent: {
            if (root.renderer === "svg") {
                return svgComponent
            }
            if (root.renderer === "gif" || root.renderer === "apng") {
                return animatedImageComponent
            }
            if (root.renderer === "png-sequence") {
                return sequenceComponent
            }
            return qmlComponent
        }
    }

    Component {
        id: qmlComponent

        Loader {
            anchors.fill: parent
            source: root.source.length > 0
                    ? appController.resolvePetResourceForPet(root.petId, root.source)
                    : "PetBody.qml"

            onLoaded: {
                if ("pressed" in item) {
                    item.pressed = Qt.binding(() => root.pressed)
                }
                if ("action" in item) {
                    item.action = Qt.binding(() => root.action)
                }
            }
        }
    }

    Component {
        id: svgComponent

        Image {
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            smooth: true
            source: appController.resolvePetResourceForPet(root.petId, appController.petActionSource(root.petId, root.action))
        }
    }

    Component {
        id: animatedImageComponent

        AnimatedImage {
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            playing: true
            source: appController.resolvePetResourceForPet(root.petId, appController.petActionSource(root.petId, root.action))
        }
    }

    Component {
        id: sequenceComponent

        PetSequence2D {
            anchors.fill: parent
            petId: root.petId
            action: root.action
            fps: root.fps
        }
    }
}
