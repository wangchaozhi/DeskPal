import QtQuick

QtObject {
    property PetWindow petWindow: PetWindow {
    }

    property PetSettingsWindow settingsWindow: PetSettingsWindow {
    }

    property Connections appConnections: Connections {
        target: appController

        function onSettingsRequested() {
            settingsWindow.show()
            settingsWindow.raise()
            settingsWindow.requestActivate()
        }
    }
}
