cmake_minimum_required(VERSION 3.0)
project(framework-example)

set(PROJECT_NAME_LIB ${PROJECT_NAME}-lib)
set(APPLICATION_RESOURCES_DIR ${PROJECT_SOURCE_DIR}/assets CACHE PATH "")

if (APPLE)
    set(PROJECT_ICON resources/settings-icon.icns)
    set_source_files_properties(resources/settings-icon.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")
endif()

configure_file(
    ${PROJECT_SOURCE_DIR}/configuration/Config.hpp.in
    ${PROJECT_BINARY_DIR}/configuration/Config.hpp
)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/configuration
)

add_library(${PROJECT_NAME_LIB}
    source/Application.cpp
    source/Resources.cpp
)

target_link_libraries(${PROJECT_NAME_LIB}
    PUBLIC engine
)

add_executable(${PROJECT_NAME} WIN32
    source/Main.cpp
    ${PROJECT_ICON}
)

target_link_libraries(${PROJECT_NAME}
    ${PROJECT_NAME_LIB}
)

set(PROJECT_COMPILE_FEATURES
    ${PROJECT_COMPILE_FEATURES}
    cxx_auto_type
    cxx_nullptr
    cxx_range_for
)

target_compile_features(${PROJECT_NAME}
    PRIVATE ${PROJECT_COMPILE_FEATURES}
)

if (APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RESOURCE resources/settings-icon.icns
        MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
        MACOSX_BUNDLE_ICON_FILE settings-icon.icns
        MACOSX_BUNDLE_INFO_PLIST
            "${PROJECT_SOURCE_DIR}/CMake/MacOSXBundleInfo.plist"
    )
endif()

