import QtQuick
import QtQuick3D

Item {
    id: root

    property bool pressed: false
    property string action: "idle"
    property string petId: appController.currentPetId
    property string renderer: "quick3d"
    property string source: ""
    property string animationClip: appController.petAnimationClip(petId, action)

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
            source: root.source.length > 0
                    ? appController.resolvePetResourceForPet(root.petId, root.source)
                    : "PetModel3D.qml"

            onLoaded: {
                if ("pressed" in item) {
                    item.pressed = Qt.binding(() => root.pressed)
                }
                if ("action" in item) {
                    item.action = Qt.binding(() => root.action)
                }
                if ("animationClip" in item) {
                    item.animationClip = Qt.binding(() => root.animationClip)
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
                    id: petModel
                    source: appController.resolvePetResourceForPet(root.petId, root.source)
                    scale: Qt.vector3d(1, 1, 1)

                    // Animation clip name mapped from pet.json "animations".
                    // Quick3D QML pets read it via the animationClip property;
                    // raw GLB/GLTF playback hooks onto this value.
                    property string activeClip: root.animationClip
                }
            }
        }
    }
}
