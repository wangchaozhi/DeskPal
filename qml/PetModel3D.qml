import QtQuick
import QtQuick3D

Item {
    id: root

    property bool pressed: false
    property string action: "idle"
    property string animationClip: ""

    width: 190
    height: 220

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Transparent
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 90, 360)
            eulerRotation.x: -12
        }

        DirectionalLight {
            eulerRotation.x: -38
            eulerRotation.y: 28
            brightness: 1.4
        }

        PointLight {
            position: Qt.vector3d(-120, 120, 180)
            brightness: 80
        }

        Node {
            id: petNode
            y: root.action === "happy" ? 18 : root.action === "sleepy" ? -8 : 0
            scale: root.action === "happy" ? Qt.vector3d(1.1, 1.1, 1.1)
                   : root.action === "sleepy" ? Qt.vector3d(1.05, 0.82, 1.05)
                   : root.pressed ? Qt.vector3d(0.96, 1.04, 0.96)
                   : Qt.vector3d(1, 1, 1)

            SequentialAnimation on eulerRotation.z {
                running: root.action === "walking"
                loops: Animation.Infinite
                NumberAnimation { to: 6; duration: 260; easing.type: Easing.InOutSine }
                NumberAnimation { to: -6; duration: 260; easing.type: Easing.InOutSine }
                NumberAnimation { to: 0; duration: 130; easing.type: Easing.InOutSine }
            }

            SequentialAnimation on eulerRotation.y {
                loops: Animation.Infinite
                running: root.action !== "dragging"
                NumberAnimation { to: -7; duration: root.action === "happy" ? 360 : 1200; easing.type: Easing.InOutSine }
                NumberAnimation { to: 7; duration: root.action === "happy" ? 360 : 1200; easing.type: Easing.InOutSine }
            }

            Model {
                id: body
                source: "#Sphere"
                scale: Qt.vector3d(1.05, 1.22, 1.05)
                materials: PrincipledMaterial {
                    baseColor: root.action === "sleepy" ? "#d8d3eb"
                               : root.action === "happy" ? "#ffd966"
                               : root.action === "dragging" ? "#f2b84b"
                               : "#f7cf5e"
                    roughness: 0.75
                }
            }

            Model {
                source: "#Sphere"
                position: Qt.vector3d(-44, 22, 82)
                scale: root.action === "sleepy" ? Qt.vector3d(0.16, 0.035, 0.04) : Qt.vector3d(0.16, 0.22, 0.04)
                materials: PrincipledMaterial { baseColor: "#2f2519"; roughness: 0.7 }
            }

            Model {
                source: "#Sphere"
                position: Qt.vector3d(44, 22, 82)
                scale: root.action === "sleepy" ? Qt.vector3d(0.16, 0.035, 0.04) : Qt.vector3d(0.16, 0.22, 0.04)
                materials: PrincipledMaterial { baseColor: "#2f2519"; roughness: 0.7 }
            }

            Model {
                source: "#Sphere"
                position: Qt.vector3d(0, -28, 88)
                scale: root.action === "happy" ? Qt.vector3d(0.28, 0.13, 0.05) : Qt.vector3d(0.2, 0.08, 0.05)
                materials: PrincipledMaterial { baseColor: "#df7f6c"; roughness: 0.65 }
            }
        }
    }
}
