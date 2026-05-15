import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property var selectedPet: ({})
    readonly property var actionNames: ["idle", "happy", "sleepy", "dragging"]
    property var actionEditModel: buildActionModel(selectedPet.actions)

    function buildActionModel(source) {
        const result = []
        for (let i = 0; i < actionNames.length; ++i) {
            const name = actionNames[i]
            result.push({ "name": name, "value": source && source[name] ? source[name] : "" })
        }
        return result
    }

    function resetFields() {
        fpsBox.value = selectedPet.fps || 0
        actionEditModel = buildActionModel(selectedPet.actions)
    }

    function payload() {
        const actions = {}
        for (let i = 0; i < actionEditModel.length; ++i) {
            actions[actionEditModel[i].name] = actionEditModel[i].value
        }
        return {
            "fps": fpsBox.value,
            "actions": actions
        }
    }

    onSelectedPetChanged: resetFields()
    Component.onCompleted: resetFields()

    Layout.fillWidth: true
    Layout.preferredHeight: form.implicitHeight + 36
    radius: 10
    color: "#ffffff"
    border.color: "#d7dce5"
    visible: selectedPet.type !== "3d"

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 4
        radius: 10
        color: "#10b981"
    }

    ColumnLayout {
        id: form
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.topMargin: 16
        anchors.bottomMargin: 18
        spacing: 12

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
