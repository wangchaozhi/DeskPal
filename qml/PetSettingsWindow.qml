import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: root

    width: 760
    height: 520
    minimumWidth: 680
    minimumHeight: 460
    visible: false
    title: qsTr("Pet Settings")
    color: "#f7f8fb"

    property var pets: appController.petProfiles()
    property int selectedIndex: findCurrentPetIndex()
    property var selectedPet: selectedIndex >= 0 && selectedIndex < pets.length ? pets[selectedIndex] : ({})

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

    Connections {
        target: appController

        function onCurrentPetChanged() {
            root.refreshPets()
        }

        function onLanguageChanged() {
            root.refreshPets()
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
                text: qsTr("Refresh")
                onClicked: root.refreshPets()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 14

            Rectangle {
                Layout.preferredWidth: 260
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

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
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
                    Layout.fillHeight: true
                    radius: 8
                    color: "#ffffff"
                    border.color: "#d7dce5"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 10

                        Label {
                            text: qsTr("Resource")
                            font.bold: true
                            color: "#1f2937"
                        }

                        TextArea {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            readOnly: true
                            wrapMode: TextEdit.WrapAnywhere
                            text: root.selectedPet.basePath && root.selectedPet.basePath.length > 0
                                  ? root.selectedPet.basePath + "\n\n" + (root.selectedPet.issueText || "")
                                  : qsTr("Built-in pet") + "\n\n" + (root.selectedPet.issueText || "")
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10

                            Button {
                                text: qsTr("Idle")
                                onClicked: appController.triggerPetAction("idle", 900)
                            }

                            Button {
                                text: qsTr("Happy")
                                onClicked: appController.triggerPetAction("happy", 1600)
                            }

                            Button {
                                text: qsTr("Sleepy")
                                onClicked: appController.triggerPetAction("sleepy", 1800)
                            }

                            Button {
                                text: qsTr("Dragging")
                                onClicked: appController.triggerPetAction("dragging", 1000)
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10

                            Button {
                                text: qsTr("Use This Pet")
                                enabled: root.selectedPet.id && root.selectedPet.id !== appController.currentPetId
                                onClicked: appController.currentPetId = root.selectedPet.id
                            }

                            Label {
                                Layout.fillWidth: true
                                text: root.selectedPet.id === appController.currentPetId ? qsTr("Current pet") : ""
                                color: "#2f7d32"
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: preview2DComponent

        PetAsset2D {
            width: 170
            height: 190
            action: appController.petAction
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
            action: appController.petAction
            petId: root.selectedPet.id || ""
            renderer: root.selectedPet.renderer || "quick3d"
            source: root.selectedPet.source || ""
        }
    }

    component DetailLabel: Label {
        Layout.preferredWidth: 76
        color: "#64748b"
        font.pixelSize: 12
    }

    component DetailValue: Label {
        Layout.fillWidth: true
        color: "#111827"
        elide: Text.ElideRight
    }
}
