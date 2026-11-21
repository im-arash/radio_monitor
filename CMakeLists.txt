cmake_minimum_required(VERSION 3.16)

project(faraz VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Location Positioning ShaderTools GuiPrivate)

qt_standard_project_setup(REQUIRES 6.10)

# Content of downloaded design from Figma to Qt or similar
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(appfaraz
    main.cpp
)

qt_add_qml_module(appfaraz
    URI faraz
    QML_FILES
        qml/Main.qml
        qml/ResourceParameters.qml
        qml/TopMenu.qml
        qml/SpectrumWave.qml
        qml/Waterfall.qml
        qml/MapHeatmap.qml
        qml/MaterialIcon.qml
        qml/LeftMenu.qml
        qml/MapContainer.qml
        qml/MenuButton.qml
    SOURCES
        spectrumwaveitem.h
        spectrumwaveitem.cpp
        spectrumtelemetry.h
        spectrumtelemetry.cpp
        waterfallitem.h
        waterfallitem.cpp
        heatmapitem.h
        heatmapitem.cpp
        dftelemetry.h
        dftelemetry.cpp
    RESOURCES
        resources/images/farazlogo.png
        resources/icons/MaterialSymbolsSharp.ttf
        resources/icons/MaterialSymbolsSharp.codepoints
)


qt_add_shaders(appfaraz "shaders"
    PREFIX "/shaders"
    BASE resources/shaders
    FILES
        resources/shaders/spectrum.vert
        resources/shaders/spectrum.frag
        resources/shaders/waterfall.vert
        resources/shaders/waterfall.frag
        resources/shaders/heatmap.vert
        resources/shaders/heatmap.frag
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
    PRIVATE Qt6::Quick Qt6::GuiPrivate Qt6::Positioning
)

install(TARGETS appfaraz
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
