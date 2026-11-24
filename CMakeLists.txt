cmake_minimum_required(VERSION 3.16)

project(faraz VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Location Positioning)

qt_standard_project_setup(REQUIRES 6.10)

# Content of downloaded design from Figma to Qt or similar
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(appfaraz
    main.cpp
)

set_source_files_properties(qml/theme/Theme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(appfaraz
    URI faraz
    QML_FILES
        qml/Main.qml
        # --- theme ---
        qml/theme/Theme.qml
        qml/theme/SettingsPopup.qml
        # --- components ---
        qml/components/MenuButton.qml
        qml/components/MaterialIcon.qml
        # --- panels ---
        qml/panels/LeftMenu.qml
        qml/panels/TopMenu.qml
        qml/panels/MapContainer.qml
        qml/panels/MapHeatmap.qml
        qml/panels/ResourceParameters.qml
        qml/panels/SpectrumWave.qml
        qml/panels/Waterfall.qml

    SOURCES
        # --- backend ---
        src/backend/packet.h
        src/backend/tcpclient.h
        src/backend/tcpclient.cpp
        src/backend/stationstate.h
        src/backend/stationstate.cpp
        src/backend/stationmodel.h
        src/backend/stationmodel.cpp
        # --- QML items (painted / rendered) ---
        src/items/heatmapitem.h
        src/items/heatmapitem.cpp
        src/items/spectrumwaveitem.h
        src/items/spectrumwaveitem.cpp
        src/items/waterfallitem.h
        src/items/waterfallitem.cpp

    RESOURCES
        resources/images/farazlogo.png
        resources/icons/MaterialSymbolsSharp.ttf
        resources/icons/MaterialSymbolsSharp.codepoints
)

target_include_directories(appfaraz PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backend
    ${CMAKE_CURRENT_SOURCE_DIR}/src/items
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appfaraz PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appfaraz
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appfaraz
    PRIVATE Qt6::Quick Qt6::Positioning
)

install(TARGETS appfaraz
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
