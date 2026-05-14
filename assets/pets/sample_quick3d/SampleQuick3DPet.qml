import QtQuick
import QtQuick3D

Item {
    id: root

    property bool pressed: false
    property string action: "idle"

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
            position: Qt.vector3d(0, 90, 360)
            eulerRotation.x: -12
        }

        DirectionalLight {
            eulerRotation.x: -38
            eulerRotation.y: 28
            brightness: 1.5
        }

        Node {
            id: petNode
            y: root.action === "happy" ? 18 : root.action === "sleepy" ? -8 : 0
            scale: root.action === "happy" ? Qt.vector3d(1.12, 1.12, 1.12)
                   : root.action === "sleepy" ? Qt.vector3d(1.05, 0.8, 1.05)
                   : root.pressed ? Qt.vector3d(0.96, 1.04, 0.96)
                   : Qt.vector3d(1, 1, 1)

            SequentialAnimation on eulerRotation.y {
                loops: Animation.Infinite
                running: root.action !== "dragging"
                NumberAnimation { to: -9; duration: root.action === "happy" ? 320 : 1100; easing.type: Easing.InOutSine }
                NumberAnimation { to: 9; duration: root.action === "happy" ? 320 : 1100; easing.type: Easing.InOutSine }
            }

            Model {
                source: "#Cube"
                scale: Qt.vector3d(1.15, 1.15, 1.15)
                materials: PrincipledMaterial {
                    baseColor: root.action === "sleepy" ? "#9fb3c8"
                               : root.action === "happy" ? "#7dd3fc"
                               : root.action === "dragging" ? "#f9a8d4"
                               : "#86efac"
                    roughness: 0.62
                }
            }

            Model {
                source: "#Sphere"
                position: Qt.vector3d(-42, 24, 86)
                scale: Qt.vector3d(0.14, root.action === "sleepy" ? 0.035 : 0.14, 0.035)
                materials: PrincipledMaterial { baseColor: "#111827"; roughness: 0.7 }
            }

            Model {
                source: "#Sphere"
                position: Qt.vector3d(42, 24, 86)
                scale: Qt.vector3d(0.14, root.action === "sleepy" ? 0.035 : 0.14, 0.035)
                materials: PrincipledMaterial { baseColor: "#111827"; roughness: 0.7 }
            }
        }
    }
}
