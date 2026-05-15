import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

AccentCard {
    id: root

    property var selectedPet: ({})
    readonly property var actionNames: ["idle", "happy", "sleepy", "dragging"]
    property var actionEditModel: buildActionModel(selectedPet.actions)

    signal browseAssetRequested(var filters, bool isFolder, var callback)

    function filtersForRenderer(r) {
        if (r === "qml") return [qsTr("QML files (*.qml)")]
        if (r === "svg") return [qsTr("SVG files (*.svg)")]
        if (r === "gif") return [qsTr("GIF files (*.gif)")]
        if (r === "apng") return [qsTr("PNG/APNG files (*.png *.apng)")]
        return [qsTr("All files (*)")]
    }

    function browseForRenderer(setter) {
        const r = selectedPet.renderer || ""
        const isFolder = (r === "png-sequence")
        root.browseAssetRequested(filtersForRenderer(r), isFolder, setter)
    }

    function buildActionModel(source) {
        const result = []
        for (let i = 0; i < actionNames.length; ++i) {
            const name = actionNames[i]
            result.push({ "name": name, "value": source && source[name] ? source[name] : "" })
        }
        return result
    }

    function resetFields() {
        fpsBox.value = selectedPet.pet2d && selectedPet.pet2d.fps !== undefined
                ? selectedPet.pet2d.fps
                : 0
        actionEditModel = buildActionModel(selectedPet.actions)
    }

    function payload() {
        const actions = {}
        for (let i = 0; i < actionEditModel.length; ++i) {
            actions[actionEditModel[i].name] = actionEditModel[i].value
        }
        return {
            "pet2d": { "fps": fpsBox.value },
            "actions": actions
        }
    }

    onSelectedPetChanged: resetFields()
    Component.onCompleted: resetFields()

    Layout.fillWidth: true
    accentColor: "#10b981"
    visible: selectedPet.type !== "3d"

        Label {
            text: qsTr("2D Settings")
            font.bold: true
            font.pixelSize: 14
            color: "#1f2937"
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 4
            columnSpacing: 12
            rowSpacing: 8
            enabled: root.selectedPet.editable === true

            DetailLabel { text: qsTr("Frame Rate") }
            SpinBox {
                id: fpsBox
                Layout.fillWidth: true
                from: 0
                to: 120
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

    component DetailLabel: Label {
        Layout.preferredWidth: 84
        color: "#64748b"
        font.pixelSize: 12
    }
}
