import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Window {
    id: root

    width: 880
    height: 640
    minimumWidth: 780
    minimumHeight: 560
    visible: false
    title: qsTr("Pet Settings")
    color: "#f7f8fb"

    property var pets: appController.petProfiles()
    property int selectedIndex: findCurrentPetIndex()
    property var selectedPet: selectedIndex >= 0 && selectedIndex < pets.length ? pets[selectedIndex] : ({})
    property string previewAction: "idle"
    property string statusMessage: ""

    readonly property var rendererOptions2D: ["qml", "svg", "gif", "apng", "png-sequence"]
    readonly property var rendererOptions3D: ["quick3d", "glb", "gltf"]
    readonly property var actionNames: ["idle", "happy", "sleepy", "dragging"]

    function findCurrentPetIndex() {
        for (let i = 0; i < pets.length; ++i) {
            if (pets[i].id === appController.currentPetId) {
                return i
            }
        }
        return pets.length > 0 ? 0 : -1
    }

    function refreshPets() {
        pets = appController.reloadPetProfiles()
        selectedIndex = findCurrentPetIndex()
    }

    function setPreview(action, duration) {
        previewAction = action
        previewRestoreTimer.interval = duration
        previewRestoreTimer.restart()
    }

    // Rebuilt whenever the selected pet changes so editor delegates reset cleanly.
    property var actionEditModel: buildActionModel(selectedPet.actions)
    property var animationEditModel: buildActionModel(selectedPet.animations)

    function buildActionModel(source) {
        const result = []
        for (let i = 0; i < actionNames.length; ++i) {
            const name = actionNames[i]
            result.push({ "name": name, "value": source && source[name] ? source[name] : "" })
        }
        return result
    }

    onSelectedPetChanged: resetEditFields()

    function resetEditFields() {
        nameField.text = selectedPet.name || ""
        sourceField.text = selectedPet.source || ""
        widthBox.value = selectedPet.width || 0
        heightBox.value = selectedPet.height || 0
        fpsBox.value = selectedPet.fps || 0
        scaleBox.value = Math.round((selectedPet.scale || 1.0) * 100)
        const options = selectedPet.type === "3d" ? rendererOptions3D : rendererOptions2D
        rendererBox.currentIndex = Math.max(0, options.indexOf(selectedPet.renderer || ""))
    }

    Timer {
        id: previewRestoreTimer
        interval: 1200
        onTriggered: root.previewAction = "idle"
    }

    Connections {
        target: appController

        function onCurrentPetChanged() {
            root.refreshPets()
        }

        function onLanguageChanged() {
            root.refreshPets()
        }
    }

    FolderDialog {
        id: importDialog
        title: qsTr("Select a pet pack folder")
        onAccepted: {
            if (appController.importPetPack(selectedFolder)) {
                root.statusMessage = qsTr("Pet pack imported")
                root.refreshPets()
            } else {
                root.statusMessage = appController.lastError
            }
        }
    }

    FolderDialog {
        id: exportDialog
        title: qsTr("Select an export destination")
        onAccepted: {
            if (appController.exportPetPack(root.selectedPet.id, selectedFolder)) {
                root.statusMessage = qsTr("Pet pack exported")
            } else {
                root.statusMessage = appController.lastError
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Label {
                text: qsTr("Pet Settings")
                font.pixelSize: 22
                font.bold: true
                color: "#1f2937"
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Import Pet Pack")
                onClicked: importDialog.open()
            }

            Button {
                text: qsTr("Export Pet Pack")
                enabled: root.selectedPet.editable === true
                onClicked: exportDialog.open()
            }

            Button {
                text: qsTr("Refresh")
                onClicked: root.refreshPets()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 18

            Label {
                text: qsTr("Opacity")
                color: "#64748b"
                font.pixelSize: 12
            }

            Slider {
                Layout.preferredWidth: 160
                from: 0.2
                to: 1.0
                value: appController.petOpacity
                onMoved: appController.petOpacity = value
            }

            CheckBox {
                text: qsTr("Start with system")
                checked: appController.autoStart
                onToggled: appController.autoStart = checked
            }

            Label {
                Layout.fillWidth: true
                text: root.statusMessage
                color: "#2f7d32"
                elide: Text.ElideRight
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 14

            Rectangle {
                Layout.preferredWidth: 240
                Layout.fillHeight: true
                radius: 8
                color: "#ffffff"
                border.color: "#d7dce5"

                ListView {
                    id: petList
                    anchors.fill: parent
                    anchors.margins: 8
                    clip: true
                    model: root.pets
                    currentIndex: root.selectedIndex

                    delegate: ItemDelegate {
                        width: petList.width
                        height: 58
                        highlighted: index === root.selectedIndex
                        onClicked: root.selectedIndex = index

                        contentItem: Column {
                            spacing: 4

                            Label {
                                width: parent.width
                                text: modelData.name
                                color: "#111827"
                                font.bold: true
                                elide: Text.ElideRight
                            }

                            Label {
                                width: parent.width
                                text: (modelData.isValid ? qsTr("Ready") : qsTr("Needs attention")) + " - " + modelData.type.toUpperCase() + " / " + modelData.renderer
                                color: modelData.isValid ? "#64748b" : "#b45309"
                                font.pixelSize: 12
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                contentWidth: availableWidth

                ColumnLayout {
                    width: parent.width
                    spacing: 12

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 210
                        radius: 8
                        color: "#ffffff"
                        border.color: "#d7dce5"

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 16

                            Rectangle {
                                Layout.preferredWidth: 210
                                Layout.fillHeight: true
                                radius: 8
                                color: "#eef2f7"
                                border.color: "#d7dce5"

                                Loader {
                                    anchors.centerIn: parent
                                    sourceComponent: root.selectedPet.type === "3d" ? preview3DComponent : preview2DComponent
                                }
                            }

                            GridLayout {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                columns: 2
                                columnSpacing: 12
                                rowSpacing: 8

                                DetailLabel { text: qsTr("Name") }
                                DetailValue { text: root.selectedPet.name || "" }

                                DetailLabel { text: qsTr("ID") }
                                DetailValue { text: root.selectedPet.id || "" }

                                DetailLabel { text: qsTr("Type") }
                                DetailValue { text: root.selectedPet.type || "" }

                                DetailLabel { text: qsTr("Renderer") }
                                DetailValue { text: root.selectedPet.renderer || "" }

                                DetailLabel { text: qsTr("Source") }
                                DetailValue { text: root.selectedPet.source || "" }

                                DetailLabel { text: qsTr("Actions") }
                                DetailValue { text: root.selectedPet.actionText || "" }

                                DetailLabel { text: qsTr("Status") }
                                DetailValue {
                                    text: root.selectedPet.isValid ? qsTr("Ready") : qsTr("Needs attention")
                                    color: root.selectedPet.isValid ? "#2f7d32" : "#b45309"
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: previewControls.implicitHeight + 28
                        radius: 8
                        color: "#ffffff"
                        border.color: "#d7dce5"

                        ColumnLayout {
                            id: previewControls
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10

                            Label {
                                text: qsTr("Action Preview")
                                font.bold: true
                                color: "#1f2937"
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Button {
                                    text: qsTr("Idle")
                                    onClicked: root.setPreview("idle", 900)
                                }

                                Button {
                                    text: qsTr("Happy")
                                    onClicked: root.setPreview("happy", 1600)
                                }

                                Button {
                                    text: qsTr("Sleepy")
                                    onClicked: root.setPreview("sleepy", 1800)
                                }

                                Button {
                                    text: qsTr("Dragging")
                                    onClicked: root.setPreview("dragging", 1000)
                                }

                                Item { Layout.fillWidth: true }

                                Button {
                                    text: qsTr("Use This Pet")
                                    enabled: root.selectedPet.id && root.selectedPet.id !== appController.currentPetId
                                    onClicked: appController.currentPetId = root.selectedPet.id
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: editForm.implicitHeight + 28
                        radius: 8
                        color: "#ffffff"
                        border.color: "#d7dce5"

                        ColumnLayout {
                            id: editForm
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10

                            RowLayout {
                                Layout.fillWidth: true

                                Label {
                                    text: qsTr("Configuration")
                                    font.bold: true
                                    color: "#1f2937"
                                    Layout.fillWidth: true
                                }

                                Label {
                                    text: root.selectedPet.editable === true
                                          ? qsTr("Editing %1").arg(root.selectedPet.id || "")
                                          : qsTr("Built-in pet is read-only")
                                    color: "#64748b"
                                    font.pixelSize: 12
                                }
                            }

                            GridLayout {
                                Layout.fillWidth: true
                                columns: 4
                                columnSpacing: 12
                                rowSpacing: 8
                                enabled: root.selectedPet.editable === true

                                DetailLabel { text: qsTr("Name") }
                                TextField {
                                    id: nameField
                                    Layout.fillWidth: true
                                    Layout.columnSpan: 3
                                    text: root.selectedPet.name || ""
                                }

                                DetailLabel { text: qsTr("Renderer") }
                                ComboBox {
                                    id: rendererBox
                                    Layout.fillWidth: true
                                    model: root.selectedPet.type === "3d" ? root.rendererOptions3D : root.rendererOptions2D
                                    currentIndex: Math.max(0, model.indexOf(root.selectedPet.renderer || ""))
                                }

                                DetailLabel { text: qsTr("Source") }
                                TextField {
                                    id: sourceField
                                    Layout.fillWidth: true
                                    text: root.selectedPet.source || ""
                                }

                                DetailLabel { text: qsTr("Width") }
                                SpinBox {
                                    id: widthBox
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 2000
                                    value: root.selectedPet.width || 0
                                }

                                DetailLabel { text: qsTr("Height") }
                                SpinBox {
                                    id: heightBox
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 2000
                                    value: root.selectedPet.height || 0
                                }

                                DetailLabel { text: qsTr("Frame Rate") }
                                SpinBox {
                                    id: fpsBox
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 120
                                    value: root.selectedPet.fps || 0
                                }

                                DetailLabel { text: qsTr("Scale %") }
                                SpinBox {
                                    id: scaleBox
                                    Layout.fillWidth: true
                                    from: 10
                                    to: 400
                                    value: Math.round((root.selectedPet.scale || 1.0) * 100)
                                }
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
                                            Layout.fillWidth: true
                                            text: modelData.value
                                            onTextChanged: modelData.value = text
                                        }
                                    }
                                }
                            }

                            Label {
                                text: qsTr("Animation Clips (3D)")
                                font.bold: true
                                color: "#1f2937"
                                visible: root.selectedPet.type === "3d"
                            }

                            GridLayout {
                                Layout.fillWidth: true
                                columns: 4
                                columnSpacing: 12
                                rowSpacing: 8
                                visible: root.selectedPet.type === "3d"
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
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Button {
                                    text: qsTr("Save Configuration")
                                    enabled: root.selectedPet.editable === true
                                    onClicked: root.saveConfiguration()
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: root.selectedPet.issueText || ""
                                    color: root.selectedPet.isValid ? "#2f7d32" : "#b45309"
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    function saveConfiguration() {
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

        const payload = {
            "id": selectedPet.id,
            "name": nameField.text,
            "renderer": rendererBox.currentText,
            "source": sourceField.text,
            "width": widthBox.value,
            "height": heightBox.value,
            "fps": fpsBox.value,
            "scale": scaleBox.value / 100.0,
            "actions": actions,
            "animations": animations
        }

        if (appController.savePetProfile(payload)) {
            statusMessage = qsTr("Configuration saved")
            refreshPets()
        } else {
            statusMessage = appController.lastError
        }
    }

    Component {
        id: preview2DComponent

        PetAsset2D {
            width: 170
            height: 190
            action: root.previewAction
            petId: root.selectedPet.id || ""
            renderer: root.selectedPet.renderer || "qml"
            source: root.selectedPet.source || ""
        }
    }

    Component {
        id: preview3DComponent

        PetAsset3D {
            width: 180
            height: 200
            action: root.previewAction
            petId: root.selectedPet.id || ""
            renderer: root.selectedPet.renderer || "quick3d"
            source: root.selectedPet.source || ""
        }
    }

    component DetailLabel: Label {
        Layout.preferredWidth: 84
        color: "#64748b"
        font.pixelSize: 12
    }

    component DetailValue: Label {
        Layout.fillWidth: true
        color: "#111827"
        elide: Text.ElideRight
    }
}
