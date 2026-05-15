import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property var selectedPet: ({})
    property string previewAction: "idle"
    property var view3dOverride: null

    signal previewRequested(string action, int duration)
    signal useRequested()

    Layout.fillWidth: true
    Layout.preferredHeight: 330
    radius: 10
    color: "#ffffff"
    border.color: "#d7dce5"

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 4
        radius: 10
        color: "#0ea5e9"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.topMargin: 16
        anchors.bottomMargin: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
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

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: qsTr("Action Preview")
                font.bold: true
                color: "#1f2937"
            }

            Button {
                text: qsTr("Idle")
                onClicked: root.previewRequested("idle", 900)
            }

            Button {
                text: qsTr("Happy")
                onClicked: root.previewRequested("happy", 1600)
            }

            Button {
                text: qsTr("Sleepy")
                onClicked: root.previewRequested("sleepy", 1800)
            }

            Button {
                text: qsTr("Dragging")
                onClicked: root.previewRequested("dragging", 1000)
            }

            Item { Layout.fillWidth: true }

            Button {
                text: qsTr("Use This Pet")
                enabled: root.selectedPet.id && root.selectedPet.id !== appController.currentPetId
                onClicked: root.useRequested()
            }
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
            view3d: root.view3dOverride !== null ? root.view3dOverride : (root.selectedPet.view3d || ({}))
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
