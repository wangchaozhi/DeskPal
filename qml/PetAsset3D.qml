import QtQuick
import QtQuick3D
import QtQuick3D.AssetUtils

Item {
    id: root

    property bool pressed: false
    property string action: "idle"
    property string petId: appController.currentPetId
    property string renderer: "quick3d"
    property string source: ""
    property string animationClip: appController.petAnimationClip(petId, action)
    property var view3d: {
        // Touch currentPetId so the binding refreshes when the active pet
        // profile is saved (savePetProfile emits currentPetChanged).
        appController.currentPetId
        return appController.petView3d(petId)
    }

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
                if ("view3d" in item) {
                    item.view3d = Qt.binding(() => root.view3d)
                }
            }
        }
    }

    Component {
        id: modelFileComponent

        View3D {
            id: assetView
            anchors.fill: parent

            environment: SceneEnvironment {
                backgroundMode: SceneEnvironment.Transparent
                antialiasingMode: SceneEnvironment.MSAA
                antialiasingQuality: SceneEnvironment.High
            }

            PerspectiveCamera {
                position: Qt.vector3d(0,
                                      root.view3d.cameraHeight !== undefined ? root.view3d.cameraHeight : 100,
                                      root.view3d.cameraDistance !== undefined ? root.view3d.cameraDistance : 360)
                eulerRotation.x: root.view3d.cameraPitch !== undefined ? root.view3d.cameraPitch : -12
            }

            DirectionalLight {
                eulerRotation.x: root.view3d.lightPitch !== undefined ? root.view3d.lightPitch : -38
                eulerRotation.y: root.view3d.lightYaw !== undefined ? root.view3d.lightYaw : 28
                brightness: root.view3d.lightBrightness !== undefined ? root.view3d.lightBrightness : 1.4
            }

            Node {
                id: transformNode
                position: Qt.vector3d(root.view3d.modelPositionX !== undefined ? root.view3d.modelPositionX : 0,
                                      root.view3d.modelPositionY !== undefined ? root.view3d.modelPositionY : 0,
                                      root.view3d.modelPositionZ !== undefined ? root.view3d.modelPositionZ : 0)
                eulerRotation: Qt.vector3d(root.view3d.modelRotationX !== undefined ? root.view3d.modelRotationX : 0,
                                           root.view3d.modelRotationY !== undefined ? root.view3d.modelRotationY : 0,
                                           root.view3d.modelRotationZ !== undefined ? root.view3d.modelRotationZ : 0)

                Node {
                    id: petNode
                    y: root.action === "happy" ? 18 : root.action === "sleepy" ? -8 : 0
                    scale: root.pressed ? Qt.vector3d(0.96, 1.04, 0.96) : Qt.vector3d(1, 1, 1)

                    SequentialAnimation on eulerRotation.y {
                        loops: Animation.Infinite
                        running: assetLoader.status !== RuntimeLoader.Success && root.action !== "dragging"
                        NumberAnimation { to: -7; duration: root.action === "happy" ? 360 : 1200; easing.type: Easing.InOutSine }
                        NumberAnimation { to: 7; duration: root.action === "happy" ? 360 : 1200; easing.type: Easing.InOutSine }
                    }

                    RuntimeLoader {
                        id: assetLoader
                        source: appController.resolvePetResourceForPet(root.petId, root.source)

                        onStatusChanged: assetView.applyClip()
                    }
                }
            }

            Connections {
                target: root
                function onAnimationClipChanged() { assetView.applyClip() }
            }

            function applyClip() {
                if (assetLoader.status !== RuntimeLoader.Success) {
                    return
                }
                const target = root.animationClip
                visitAnimations(assetLoader, function(timeline, anim) {
                    const timelineName = timeline.objectName || ""
                    const animName = anim.objectName || ""
                    const matches = target.length > 0
                                    && (timelineName === target || animName === target)
                    if ("enabled" in timeline) {
                        timeline.enabled = target.length === 0 || matches
                    }
                    if ("running" in anim) {
                        anim.running = matches
                    }
                    if ("loops" in anim) {
                        anim.loops = Animation.Infinite
                    }
                })
            }

            function visitAnimations(node, fn) {
                if (!node) {
                    return
                }
                if (node.animations !== undefined && node.animations.length !== undefined) {
                    for (var i = 0; i < node.animations.length; ++i) {
                        try {
                            fn(node, node.animations[i])
                        } catch (e) {
                            // Some QQuick3DObject children expose an animations
                            // property that isn't a TimelineAnimation list; ignore.
                        }
                    }
                }
                if (node.children !== undefined && node.children.length !== undefined) {
                    for (var c = 0; c < node.children.length; ++c) {
                        visitAnimations(node.children[c], fn)
                    }
                }
            }
        }
    }
}
