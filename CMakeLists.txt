cmake_minimum_required(VERSION 3.12)

project(QRhiTutorial CXX)

set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

function(add_example SUB_DIR)
    get_filename_component(ProjectId ${SUB_DIR} NAME)
    file(GLOB PROJECT_SOURCE FILES ${CMAKE_CURRENT_LIST_DIR}/${SUB_DIR}/*.h  ${CMAKE_CURRENT_LIST_DIR}/${SUB_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/${SUB_DIR}/*.qrc)
    file(GLOB SHADER_SOURCE FILES   ${CMAKE_CURRENT_LIST_DIR}/${SUB_DIR}/*.vert  ${SUB_DIR}/*.frag   ${CMAKE_CURRENT_LIST_DIR}/${SUB_DIR}/*.comp)
    source_group("Shader Files/GLSL" FILES ${SHADER_SOURCE})
    source_group("Source Files" FILES ${PROJECT_SOURCE})
    add_executable(${ProjectId} 
        ${PROJECT_SOURCE}
        ${SHADER_SOURCE}
    )

    set(PROJECT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ProjectId})
    set(PROJECT_SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ProjectId}/Shaders)

    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR})
    file(MAKE_DIRECTORY ${PROJECT_SHADER_DIR})

    target_include_directories(${ProjectId} PUBLIC ${PROJECT_SHADER_DIR})
    target_link_libraries(${ProjectId} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::GuiPrivate)
endfunction()

add_subdirectory(Examples)
