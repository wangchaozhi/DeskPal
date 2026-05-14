import QtQuick
import QtQuick3D

Item {
    id: root

    property bool pressed: false
    property string action: "idle"
    property string renderer: "quick3d"

    width: 190
    height: 220

    Loader {
        anchors.fill: parent
        sourceComponent: root.renderer === "glb" || root.renderer === "gltf" ? modelFileComponent : quick3DComponent
    }

    Component {
        id: quick3DComponent

        Loader {
            anchors.fill: parent
            source: appController.currentPetBasePath.length > 0
                    ? appController.resolvePetResource(appController.currentPetSource)
                    : "PetModel3D.qml"

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
        id: modelFileComponent

        View3D {
            anchors.fill: parent

            environment: SceneEnvironment {
                backgroundMode: SceneEnvironment.Transparent
                antialiasingMode: SceneEnvironment.MSAA
                antialiasingQuality: SceneEnvironment.High
            }

            PerspectiveCamera {
                position: Qt.vector3d(0, 100, 360)
                eulerRotation.x: -12
            }

            DirectionalLight {
                eulerRotation.x: -38
                eulerRotation.y: 28
                brightness: 1.4
            }

            Node {
                id: petNode
                y: root.action === "happy" ? 18 : root.action === "sleepy" ? -8 : 0
                scale: root.pressed ? Qt.vector3d(0.96, 1.04, 0.96) : Qt.vector3d(1, 1, 1)

                SequentialAnimation on eulerRotation.y {
                    loops: Animation.Infinite
                    running: root.action !== "dragging"
                    NumberAnimation { to: -7; duration: root.action === "happy" ? 360 : 1200; easing.type: Easing.InOutSine }
                    NumberAnimation { to: 7; duration: root.action === "happy" ? 360 : 1200; easing.type: Easing.InOutSine }
                }

                Model {
                    source: appController.resolvePetResource(appController.currentPetSource)
                    scale: Qt.vector3d(1, 1, 1)
                }
            }
        }
    }
}
