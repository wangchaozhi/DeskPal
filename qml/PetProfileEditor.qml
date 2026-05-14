import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property var selectedPet: ({})
    property var rendererOptions: []

    function resetFields() {
        nameField.text = selectedPet.name || ""
        sourceField.text = selectedPet.source || ""
        widthBox.value = selectedPet.width || 0
        heightBox.value = selectedPet.height || 0
        scaleBox.value = Math.round((selectedPet.scale || 1.0) * 100)
        rendererBox.currentIndex = Math.max(0, rendererOptions.indexOf(selectedPet.renderer || ""))
    }

    function profilePayload() {
        return {
            "id": selectedPet.id,
            "name": nameField.text,
            "renderer": rendererBox.currentText,
            "source": sourceField.text,
            "width": widthBox.value,
            "height": heightBox.value,
            "scale": scaleBox.value / 100.0
        }
    }

    onSelectedPetChanged: resetFields()
    onRendererOptionsChanged: resetFields()
    Component.onCompleted: resetFields()

    Layout.fillWidth: true
    Layout.preferredHeight: form.implicitHeight + 28
    radius: 8
    color: "#ffffff"
    border.color: "#d7dce5"

    ColumnLayout {
        id: form
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Profile")
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
            }

            DetailLabel { text: qsTr("Renderer") }
            ComboBox {
                id: rendererBox
                Layout.fillWidth: true
                model: root.rendererOptions
            }

            DetailLabel { text: qsTr("Source") }
            TextField {
                id: sourceField
                Layout.fillWidth: true
            }

            DetailLabel { text: qsTr("Width") }
            SpinBox {
                id: widthBox
                Layout.fillWidth: true
                from: 0
                to: 2000
            }

            DetailLabel { text: qsTr("Height") }
            SpinBox {
                id: heightBox
                Layout.fillWidth: true
                from: 0
                to: 2000
            }

            DetailLabel { text: qsTr("Scale %") }
            SpinBox {
                id: scaleBox
                Layout.fillWidth: true
                from: 10
                to: 400
            }
        }
    }

    component DetailLabel: Label {
        Layout.preferredWidth: 84
        color: "#64748b"
        font.pixelSize: 12
    }
}
