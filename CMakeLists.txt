cmake_minimum_required(VERSION 3.16)

project(HomeLauncher VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

option(USE_INSTALLED_COMMONAPI "Use installed CommonAPI" ON)
find_package(CommonAPI 3.2.4 REQUIRED CONFIG)
find_package(CommonAPI-SomeIP 3.2.4 REQUIRED)
find_package(vsomeip3 3.7.2 REQUIRED)

qt_standard_project_setup(REQUIRES 6.8)

set(PRJ_SRC_PATH src)
set(PRJ_SRC_GEN_PATH src-gen)
set(PRJ_SRC_GEN_COMMONAPI_PATH ${PRJ_SRC_GEN_PATH}/core/v1/Heimdall)
set(PRJ_SRC_GEN_COMMONAPI_SOMEIP_PATH ${PRJ_SRC_GEN_PATH}/someip/v1/Heimdall)

file(GLOB PRJ_PROXY_GEN_SRCS ${PRJ_SRC_GEN_COMMONAPI_PATH}/*Proxy*.cpp)
file(GLOB PRJ_SOMEIP_LIB_SRCS ${PRJ_SRC_GEN_COMMONAPI_SOMEIP_PATH}/*.cpp)

include_directories(
    src-gen/core
    src-gen/someip
    src
    ${COMMONAPI_INCLUDE_DIRS}
    ${COMMONAPI_SOMEIP_INCLUDE_DIRS}
    ${VSOMEIP_INCLUDE_DIRS}
)

link_directories(${COMMONAPI_LIBDIR})


qt_add_executable(appHomeLauncher
    main.cpp
    src/HeimdallBackend.cpp
    ${PRJ_PROXY_GEN_SRCS}
)

qt_add_qml_module(appHomeLauncher
    URI HomeLauncher
    QML_FILES
    Main.qml
    RESOURCES resources_file.qrc
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appHomeLauncher PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appHomeLauncher
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appHomeLauncher
    PRIVATE Qt6::Quick
    CommonAPI
    CommonAPI-SomeIP
    vsomeip3
)

add_library(Heimdall-HomeUI-someip SHARED ${PRJ_SOMEIP_LIB_SRCS})
target_link_libraries(Heimdall-HomeUI-someip CommonAPI-SomeIP)

include(GNUInstallDirs)
install(TARGETS appHomeLauncher Heimdall-HomeUI-someip
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
