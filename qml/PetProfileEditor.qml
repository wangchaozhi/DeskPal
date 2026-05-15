import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

AccentCard {
    id: root

    property var selectedPet: ({})
    property var rendererOptions: []

    signal browseAssetRequested(var filters, bool isFolder, var callback)

    function filtersForRenderer(r) {
        if (r === "qml" || r === "quick3d") return [qsTr("QML files (*.qml)")]
        if (r === "svg") return [qsTr("SVG files (*.svg)")]
        if (r === "gif") return [qsTr("GIF files (*.gif)")]
        if (r === "apng") return [qsTr("PNG/APNG files (*.png *.apng)")]
        if (r === "glb") return [qsTr("GLB files (*.glb)")]
        if (r === "gltf") return [qsTr("GLTF files (*.gltf)")]
        return [qsTr("All files (*)")]
    }

    function browseForRenderer(setter) {
        const r = (selectedPet.renderer || rendererBox.currentText) || ""
        const isFolder = (r === "png-sequence")
        root.browseAssetRequested(filtersForRenderer(r), isFolder, setter)
    }

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
    accentColor: "#3b82f6"

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Profile")
                font.bold: true
                font.pixelSize: 14
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
            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                TextField {
                    id: sourceField
                    Layout.fillWidth: true
                }
                Button {
                    text: "…"
                    Layout.preferredWidth: 36
                    enabled: root.selectedPet.editable === true
                    onClicked: root.browseForRenderer(function(rel) {
                        sourceField.text = rel
                    })
                }
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

    component DetailLabel: Label {
        Layout.preferredWidth: 84
        color: "#64748b"
        font.pixelSize: 12
    }
}
