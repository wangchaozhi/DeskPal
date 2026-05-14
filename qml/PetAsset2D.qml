import QtQuick

Item {
    id: root

    property bool pressed: false
    property string action: "idle"
    property string renderer: "qml"
    property string source: ""

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
            source: appController.currentPetBasePath.length > 0
                    ? appController.resolvePetResource(appController.currentPetSource)
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
            source: appController.resolvePetResource(appController.currentPetActionSource(root.action))
        }
    }

    Component {
        id: animatedImageComponent

        AnimatedImage {
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            playing: true
            source: appController.resolvePetResource(appController.currentPetActionSource(root.action))
        }
    }

    Component {
        id: sequenceComponent

        PetSequence2D {
            anchors.fill: parent
            action: root.action
        }
    }
}
