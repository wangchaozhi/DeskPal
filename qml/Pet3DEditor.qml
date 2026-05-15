import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

AccentCard {
    id: root

    property var selectedPet: ({})
    readonly property var actionNames: ["idle", "happy", "sleepy", "dragging"]
    property var actionEditModel: buildActionModel(selectedPet.actions)
    property var animationEditModel: buildActionModel(selectedPet.animations)

    readonly property var liveValues: ({
        "cameraDistance": cameraDistanceBox.value,
        "cameraHeight": cameraHeightBox.value,
        "cameraPitch": cameraPitchBox.value,
        "modelRotationX": rotationXBox.value,
        "modelRotationY": rotationYBox.value,
        "modelRotationZ": rotationZBox.value,
        "modelPositionX": positionXBox.value,
        "modelPositionY": positionYBox.value,
        "modelPositionZ": positionZBox.value,
        "lightBrightness": lightBrightnessBox.value / 100.0,
        "lightPitch": lightPitchBox.value,
        "lightYaw": lightYawBox.value
    })

    signal previewRequested(string action, int duration)
    signal browseAssetRequested(var filters, bool isFolder, var callback)

    function filtersForRenderer(r) {
        if (r === "quick3d") return [qsTr("QML files (*.qml)")]
        if (r === "glb") return [qsTr("GLB files (*.glb)")]
        if (r === "gltf") return [qsTr("GLTF files (*.gltf)")]
        return [qsTr("All files (*)")]
    }

    function browseForRenderer(setter) {
        const r = selectedPet.renderer || ""
        root.browseAssetRequested(filtersForRenderer(r), false, setter)
    }

    function buildActionModel(source) {
        const result = []
        for (let i = 0; i < actionNames.length; ++i) {
            const name = actionNames[i]
            result.push({ "name": name, "value": source && source[name] ? source[name] : "" })
        }
        return result
    }

    function view3dValue(key, fallback) {
        const view = selectedPet.view3d
        if (view && view[key] !== undefined) {
            return view[key]
        }
        return fallback
    }

    function resetFields() {
        actionEditModel = buildActionModel(selectedPet.actions)
        animationEditModel = buildActionModel(selectedPet.animations)

        cameraDistanceBox.value = Math.round(view3dValue("cameraDistance", 360))
        cameraHeightBox.value = Math.round(view3dValue("cameraHeight", 90))
        cameraPitchBox.value = Math.round(view3dValue("cameraPitch", -12))
        rotationXBox.value = Math.round(view3dValue("modelRotationX", 0))
        rotationYBox.value = Math.round(view3dValue("modelRotationY", 0))
        rotationZBox.value = Math.round(view3dValue("modelRotationZ", 0))
        positionXBox.value = Math.round(view3dValue("modelPositionX", 0))
        positionYBox.value = Math.round(view3dValue("modelPositionY", 0))
        positionZBox.value = Math.round(view3dValue("modelPositionZ", 0))
        lightBrightnessBox.value = Math.round(view3dValue("lightBrightness", 1.4) * 100)
        lightPitchBox.value = Math.round(view3dValue("lightPitch", -38))
        lightYawBox.value = Math.round(view3dValue("lightYaw", 28))
    }

    function liveView3d() {
        return {
            "cameraDistance": cameraDistanceBox.value,
            "cameraHeight": cameraHeightBox.value,
            "cameraPitch": cameraPitchBox.value,
            "modelRotationX": rotationXBox.value,
            "modelRotationY": rotationYBox.value,
            "modelRotationZ": rotationZBox.value,
            "modelPositionX": positionXBox.value,
            "modelPositionY": positionYBox.value,
            "modelPositionZ": positionZBox.value,
            "lightBrightness": lightBrightnessBox.value / 100.0,
            "lightPitch": lightPitchBox.value,
            "lightYaw": lightYawBox.value
        }
    }

    function payload() {
        const actions = {}
        for (let i = 0; i < actionEditModel.length; ++i) {
            actions[actionEditModel[i].name] = actionEditModel[i].value
        }

        const animations = {}
        for (let j = 0; j < animationEditModel.length; ++j) {
            if (animationEditModel[j].value.length > 0) {
                animations[animationEditModel[j].name] = animationEditModel[j].value
            }
        }

        return {
            "actions": actions,
            "animations": animations,
            "view3d": liveView3d()
        }
    }

    onSelectedPetChanged: resetFields()
    Component.onCompleted: resetFields()

    onLiveValuesChanged: {
        if (selectedPet && selectedPet.id && selectedPet.editable === true && selectedPet.type === "3d") {
            appController.setLiveView3d(selectedPet.id, liveValues)
        }
    }

    Layout.fillWidth: true
    accentColor: "#8b5cf6"
    visible: selectedPet.type === "3d"

        Label {
            text: qsTr("3D Stage")
            font.bold: true
            font.pixelSize: 14
            color: "#1f2937"
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("Adjust camera framing, model transform, and lighting. Changes apply after saving.")
            color: "#64748b"
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 4
            columnSpacing: 12
            rowSpacing: 8
            enabled: root.selectedPet.editable === true

            DetailLabel { text: qsTr("Camera Distance") }
            EditorSpinBox { id: cameraDistanceBox; from: 50; to: 2000; stepSize: 10 }

            DetailLabel { text: qsTr("Camera Height") }
            EditorSpinBox { id: cameraHeightBox; from: -500; to: 500; stepSize: 5 }

            DetailLabel { text: qsTr("Camera Pitch") }
            EditorSpinBox { id: cameraPitchBox; from: -90; to: 90 }

            DetailLabel { text: qsTr("Light Brightness %") }
            EditorSpinBox { id: lightBrightnessBox; from: 0; to: 500; stepSize: 5 }

            DetailLabel { text: qsTr("Light Pitch") }
            EditorSpinBox { id: lightPitchBox; from: -180; to: 180 }

            DetailLabel { text: qsTr("Light Yaw") }
            EditorSpinBox { id: lightYawBox; from: -180; to: 180 }

            DetailLabel { text: qsTr("Rotation X") }
            EditorSpinBox { id: rotationXBox; from: -180; to: 180 }

            DetailLabel { text: qsTr("Rotation Y") }
            EditorSpinBox { id: rotationYBox; from: -180; to: 180 }

            DetailLabel { text: qsTr("Rotation Z") }
            EditorSpinBox { id: rotationZBox; from: -180; to: 180 }

            DetailLabel { text: qsTr("Position X") }
            EditorSpinBox { id: positionXBox; from: -300; to: 300; stepSize: 2 }

            DetailLabel { text: qsTr("Position Y") }
            EditorSpinBox { id: positionYBox; from: -300; to: 300; stepSize: 2 }

            DetailLabel { text: qsTr("Position Z") }
            EditorSpinBox { id: positionZBox; from: -300; to: 300; stepSize: 2 }
        }

        Label {
            text: qsTr("Action Resources")
            font.bold: true
            color: "#1f2937"
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 4
            columnSpacing: 12
            rowSpacing: 8
            enabled: root.selectedPet.editable === true

            Repeater {
                model: root.actionEditModel
                delegate: RowLayout {
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    spacing: 8

                    DetailLabel { text: modelData.name }
                    TextField {
                        id: actionField
                        Layout.fillWidth: true
                        text: modelData.value
                        onTextChanged: modelData.value = text
                    }
                    Button {
                        text: "…"
                        Layout.preferredWidth: 36
                        enabled: root.selectedPet.editable === true
                        onClicked: root.browseForRenderer(function(rel) {
                            actionField.text = rel
                        })
                    }
                }
            }
        }

        Label {
            text: qsTr("Animation Clips")
            font.bold: true
            color: "#1f2937"
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 4
            columnSpacing: 12
            rowSpacing: 8
            enabled: root.selectedPet.editable === true

            Repeater {
                model: root.animationEditModel
                delegate: RowLayout {
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    spacing: 8

                    DetailLabel { text: modelData.name }
                    TextField {
                        Layout.fillWidth: true
                        text: modelData.value
                        onTextChanged: modelData.value = text
                    }
                    Button {
                        text: qsTr("Preview")
                        enabled: modelData.value.length > 0
                        onClicked: root.previewRequested(modelData.name, 2200)
                    }
                }
            }
        }

    component DetailLabel: Label {
        Layout.preferredWidth: 110
        color: "#64748b"
        font.pixelSize: 12
    }

    component EditorSpinBox: SpinBox {
        Layout.fillWidth: true
        editable: true
    }
}
