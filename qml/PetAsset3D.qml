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
        // Touch live override so editing in the settings window refreshes
        // the live desktop pet too.
        appController.currentLiveView3d
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

    function viewProp(key, fallback) {
        return view3d && view3d[key] !== undefined ? view3d[key] : fallback
    }

    Component {
        id: modelFileComponent

        View3D {
            id: assetView
            anchors.fill: parent
            property var animationEntries: []

            environment: SceneEnvironment {
                backgroundMode: SceneEnvironment.Transparent
                antialiasingMode: SceneEnvironment.MSAA
                antialiasingQuality: SceneEnvironment.High
            }

            PerspectiveCamera {
                position: Qt.vector3d(0, root.viewProp("cameraHeight", 100), root.viewProp("cameraDistance", 360))
                eulerRotation.x: root.viewProp("cameraPitch", -12)
            }

            DirectionalLight {
                eulerRotation.x: root.viewProp("lightPitch", -38)
                eulerRotation.y: root.viewProp("lightYaw", 28)
                brightness: root.viewProp("lightBrightness", 1.4)
            }

            Node {
                id: transformNode
                position: Qt.vector3d(root.viewProp("modelPositionX", 0),
                                      root.viewProp("modelPositionY", 0),
                                      root.viewProp("modelPositionZ", 0))
                eulerRotation: Qt.vector3d(root.viewProp("modelRotationX", 0),
                                           root.viewProp("modelRotationY", 0),
                                           root.viewProp("modelRotationZ", 0))

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

                        onStatusChanged: {
                            assetView.rebuildAnimationCache()
                            assetView.applyClip()
                        }
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
                for (let i = 0; i < animationEntries.length; ++i) {
                    const entry = animationEntries[i]
                    const matches = target.length > 0
                                    && (entry.timelineName === target || entry.animationName === target)
                    if ("enabled" in entry.timeline) {
                        entry.timeline.enabled = target.length === 0 || matches
                    }
                    if ("running" in entry.animation) {
                        entry.animation.running = matches
                    }
                    if ("loops" in entry.animation) {
                        entry.animation.loops = Animation.Infinite
                    }
                }
            }

            function rebuildAnimationCache() {
                if (assetLoader.status !== RuntimeLoader.Success) {
                    animationEntries = []
                    return
                }
                const entries = []
                visitAnimations(assetLoader, function(timeline, anim) {
                    const timelineName = timeline.objectName || ""
                    const animationName = anim.objectName || ""
                    entries.push({
                        "timeline": timeline,
                        "animation": anim,
                        "timelineName": timelineName,
                        "animationName": animationName
                    })
                    if ("loops" in anim) {
                        anim.loops = Animation.Infinite
                    }
                })
                animationEntries = entries
            }

            function visitAnimations(node, fn) {
                if (!node) {
                    return
                }
                if (node.animations !== undefined && node.animations.length !== undefined) {
                    for (let i = 0; i < node.animations.length; ++i) {
                        try {
                            fn(node, node.animations[i])
                        } catch (e) {
                            // Some QQuick3DObject children expose an animations
                            // property that isn't a TimelineAnimation list; ignore.
                        }
                    }
                }
                if (node.children !== undefined && node.children.length !== undefined) {
                    for (let c = 0; c < node.children.length; ++c) {
                        visitAnimations(node.children[c], fn)
                    }
                }
            }
        }
    }
}
