cmake_minimum_required(VERSION 3.21)
project(SabaiBooksQt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.11 REQUIRED COMPONENTS Core Gui Widgets Network)
qt_standard_project_setup(REQUIRES 6.11)

qt_add_executable(SabaiBooksQt
    main.cpp
    mainwindow.cpp
    mainwindow.h
    resources.qrc
)

target_link_libraries(SabaiBooksQt PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

if (WIN32)
    set_target_properties(SabaiBooksQt PROPERTIES WIN32_EXECUTABLE TRUE)
endif()


# Copy the bundled assets next to the executable after every build.
add_custom_command(TARGET SabaiBooksQt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets/lectures"
            "$<TARGET_FILE_DIR:SabaiBooksQt>/assets"
)
