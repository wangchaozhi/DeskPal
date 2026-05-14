cmake_minimum_required(VERSION 3.16)

project(DeskPal VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Widgets)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(DeskPal
    main.cpp
    src/AppController.cpp
    src/AppController.h
)

qt_add_qml_module(DeskPal
    URI DeskPal
    QML_FILES
        Main.qml
        qml/PetWindow.qml
        qml/PetBody.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(DeskPal PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.DeskPal
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(DeskPal
    PRIVATE
        Qt6::Quick
        Qt6::Widgets
)

install(TARGETS DeskPal
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
