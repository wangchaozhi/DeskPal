cmake_minimum_required(VERSION 3.16)

project(DeskPal VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Quick3D Quick3DAssetUtils QuickControls2 Widgets LinguistTools)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(DeskPal
    main.cpp
    src/ActionController.cpp
    src/ActionController.h
    src/AppController.cpp
    src/AppController.h
    src/PetCatalog.cpp
    src/PetCatalog.h
    src/SettingsStore.cpp
    src/SettingsStore.h
    src/TrayController.cpp
    src/TrayController.h
    src/TranslationManager.cpp
    src/TranslationManager.h
)

qt_add_qml_module(DeskPal
    URI DeskPal
    QML_FILES
        Main.qml
        qml/PetWindow.qml
        qml/PetBody.qml
        qml/PetModel3D.qml
        qml/PetAsset2D.qml
        qml/PetAsset3D.qml
        qml/PetSequence2D.qml
        qml/PetSettingsWindow.qml
        qml/PetPreviewPanel.qml
        qml/PetProfileEditor.qml
        qml/Pet2DEditor.qml
        qml/Pet3DEditor.qml
)

qt_add_resources(DeskPal "sample_pets"
    PREFIX "/samples"
    BASE "assets/pets"
    FILES
        assets/pets/sample_svg_2d/pet.json
        assets/pets/sample_svg_2d/idle.svg
        assets/pets/sample_svg_2d/happy.svg
        assets/pets/sample_svg_2d/sleepy.svg
        assets/pets/sample_svg_2d/dragging.svg
        assets/pets/sample_quick3d/pet.json
        assets/pets/sample_quick3d/SampleQuick3DPet.qml
)

qt_add_translations(DeskPal
    TS_FILES
        translations/DeskPal_zh_CN.ts
    RESOURCE_PREFIX
        /i18n
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(DeskPal PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.DeskPal
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(DeskPal
    PRIVATE
        Qt6::Quick
        Qt6::Quick3D
        Qt6::Quick3DAssetUtils
        Qt6::QuickControls2
        Qt6::Widgets
)

install(TARGETS DeskPal
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY assets
    DESTINATION .
)
