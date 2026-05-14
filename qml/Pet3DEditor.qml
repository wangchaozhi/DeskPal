import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property var selectedPet: ({})
    readonly property var actionNames: ["idle", "happy", "sleepy", "dragging"]
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

    function resetFields() {
        actionEditModel = buildActionModel(selectedPet.actions)
        animationEditModel = buildActionModel(selectedPet.animations)
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
            "animations": animations
        }
    }

    onSelectedPetChanged: resetFields()
    Component.onCompleted: resetFields()

    Layout.fillWidth: true
    Layout.preferredHeight: form.implicitHeight + 28
    radius: 8
    color: "#ffffff"
    border.color: "#d7dce5"
    visible: selectedPet.type === "3d"

    ColumnLayout {
        id: form
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        Label {
            text: qsTr("3D Settings")
            font.bold: true
            color: "#1f2937"
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("Model transform, camera, and lighting controls will live here.")
            color: "#64748b"
            font.pixelSize: 12
            wrapMode: Text.WordWrap
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
