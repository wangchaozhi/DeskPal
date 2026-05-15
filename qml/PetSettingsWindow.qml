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

    onVisibleChanged: {
        if (!visible) {
            appController.clearLiveView3d()
        }
    }

    onSelectedPetChanged: appController.clearLiveView3d()

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

    property var pendingAssetCallback: null

    FileDialog {
        id: assetFileDialog
        title: qsTr("Select an asset file")
        onAccepted: root.handleAssetSelected(selectedFile)
    }

    FolderDialog {
        id: assetFolderDialog
        title: qsTr("Select an asset folder")
        onAccepted: root.handleAssetSelected(selectedFolder)
    }

    function browseAsset(filters, isFolder, callback) {
        pendingAssetCallback = callback
        if (isFolder) {
            assetFolderDialog.open()
        } else {
            assetFileDialog.nameFilters = filters && filters.length > 0
                    ? filters
                    : [qsTr("All files (*)")]
            assetFileDialog.open()
        }
    }

    function handleAssetSelected(url) {
        if (!root.selectedPet || !root.selectedPet.id) {
            statusMessage = qsTr("No pet selected")
            return
        }
        const rel = appController.importPetAsset(root.selectedPet.id, url)
        if (rel.length > 0) {
            statusMessage = qsTr("Asset imported: %1").arg(rel)
            if (pendingAssetCallback) {
                pendingAssetCallback(rel)
            }
        } else {
            statusMessage = appController.lastError
        }
        pendingAssetCallback = null
    }

    Dialog {
        id: newPetDialog
        title: qsTr("New Pet")
        width: 340
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        onOpened: {
            newPetName.text = ""
            newPetType.currentIndex = 0
            newPetName.forceActiveFocus()
        }

        onAccepted: {
            if (appController.createPet(newPetName.text, newPetType.currentText)) {
                root.statusMessage = qsTr("Pet created")
                root.refreshPets()
            } else {
                root.statusMessage = appController.lastError
            }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            TextField {
                id: newPetName
                Layout.fillWidth: true
                placeholderText: qsTr("Pet name")
            }

            ComboBox {
                id: newPetType
                Layout.fillWidth: true
                model: ["2d", "3d"]
            }
        }
    }

    Dialog {
        id: deletePetDialog
        title: qsTr("Delete Pet")
        width: 360
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Yes | Dialog.No

        onAccepted: {
            if (appController.deletePet(root.selectedPet.id)) {
                root.statusMessage = qsTr("Pet deleted")
                root.refreshPets()
            } else {
                root.statusMessage = appController.lastError
            }
        }

        Label {
            anchors.fill: parent
            wrapMode: Text.WordWrap
            text: qsTr("Delete pet \"%1\"? Its folder will be removed from disk.").arg(root.selectedPet.name || "")
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
                text: qsTr("New Pet")
                onClicked: newPetDialog.open()
            }

            Button {
                text: qsTr("Delete")
                enabled: root.selectedPet.editable === true
                onClicked: deletePetDialog.open()
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
                id: sampleButton
                text: qsTr("Generate Sample")
                enabled: appController.availableSamplePets().length > 0
                onClicked: sampleMenu.open()

                Menu {
                    id: sampleMenu
                    y: sampleButton.height

                    Instantiator {
                        model: appController.availableSamplePets()
                        delegate: MenuItem {
                            text: modelData
                            onTriggered: {
                                if (appController.installSamplePet(modelData)) {
                                    root.statusMessage = qsTr("Sample pet installed")
                                    root.refreshPets()
                                } else {
                                    root.statusMessage = appController.lastError
                                }
                            }
                        }
                        onObjectAdded: (index, object) => sampleMenu.insertItem(index, object)
                        onObjectRemoved: (index, object) => sampleMenu.removeItem(object)
                    }
                }
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

            CheckBox {
                text: qsTr("Wander automatically")
                checked: appController.wanderEnabled
                onToggled: appController.wanderEnabled = checked
            }

            CheckBox {
                text: qsTr("Sleepy at night")
                checked: appController.nightSleepyEnabled
                onToggled: appController.nightSleepyEnabled = checked
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
                id: rightScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                contentWidth: availableWidth
                ScrollBar.vertical.policy: ScrollBar.AsNeeded

                ColumnLayout {
                    width: rightScroll.availableWidth - 46
                    x: 10
                    spacing: 16

                    PetPreviewPanel {
                        selectedPet: root.selectedPet
                        previewAction: root.previewAction
                        view3dOverride: root.selectedPet.type === "3d" ? pet3DEditor.liveValues : null
                        onPreviewRequested: (action, duration) => root.setPreview(action, duration)
                        onUseRequested: appController.currentPetId = root.selectedPet.id
                    }

                    PetProfileEditor {
                        id: profileEditor
                        selectedPet: root.selectedPet
                        rendererOptions: root.selectedPet.type === "3d" ? root.rendererOptions3D : root.rendererOptions2D
                        onBrowseAssetRequested: (filters, isFolder, callback) => root.browseAsset(filters, isFolder, callback)
                    }

                    Pet2DEditor {
                        id: pet2DEditor
                        selectedPet: root.selectedPet
                        onBrowseAssetRequested: (filters, isFolder, callback) => root.browseAsset(filters, isFolder, callback)
                    }

                    Pet3DEditor {
                        id: pet3DEditor
                        selectedPet: root.selectedPet
                        onPreviewRequested: (action, duration) => root.setPreview(action, duration)
                        onBrowseAssetRequested: (filters, isFolder, callback) => root.browseAsset(filters, isFolder, callback)
                    }

                    AccentCard {
                        Layout.fillWidth: true
                        accentColor: "#f59e0b"

                        RowLayout {
                            id: savePanel
                            Layout.fillWidth: true
                            spacing: 14

                            Button {
                                text: qsTr("Save Configuration")
                                highlighted: true
                                Layout.alignment: Qt.AlignTop
                                enabled: root.selectedPet.editable === true
                                onClicked: root.saveConfiguration()
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4

                                Label {
                                    Layout.fillWidth: true
                                    visible: (root.selectedPet.errors || []).length === 0
                                            && (root.selectedPet.warnings || []).length === 0
                                    text: qsTr("Ready")
                                    color: "#2f7d32"
                                    font.bold: true
                                }

                                Repeater {
                                    model: root.selectedPet.errors || []
                                    delegate: Label {
                                        Layout.fillWidth: true
                                        text: "● " + modelData
                                        color: "#b91c1c"
                                        wrapMode: Text.WordWrap
                                    }
                                }

                                Repeater {
                                    model: root.selectedPet.warnings || []
                                    delegate: Label {
                                        Layout.fillWidth: true
                                        text: "○ " + modelData
                                        color: "#b45309"
                                        wrapMode: Text.WordWrap
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    function saveConfiguration() {
        let payload = profileEditor.profilePayload()
        const typePayload = selectedPet.type === "3d" ? pet3DEditor.payload() : pet2DEditor.payload()
        for (const key in typePayload) {
            payload[key] = typePayload[key]
        }

        if (appController.savePetProfile(payload)) {
            statusMessage = qsTr("Configuration saved")
            refreshPets()
        } else {
            statusMessage = appController.lastError
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
