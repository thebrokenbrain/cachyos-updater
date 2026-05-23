cmake_minimum_required(VERSION 3.21)
project(cachyos-updater
    VERSION 0.1.0
    DESCRIPTION "Actualizador grafico simple para CachyOS"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

add_executable(cachyos-updater
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/updater.cpp
    src/updater.h
    resources/resources.qrc
)

target_compile_definitions(cachyos-updater PRIVATE
    CACHYOS_UPDATER_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(cachyos-updater PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

include(GNUInstallDirs)

install(TARGETS cachyos-updater
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(PROGRAMS packaging/cachyos-updater-helper.sh
    DESTINATION lib/cachyos-updater
    RENAME cachyos-updater-helper)

install(FILES packaging/cachyos-updater.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)

install(FILES packaging/cachyos-updater.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps)

install(FILES packaging/org.cachyos.updater.policy
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/polkit-1/actions)
