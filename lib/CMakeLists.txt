project(lib)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

IF (WIN32)
    MESSAGE(STATUS "Platform is Windows")
    add_definitions(-DWINDOWS)
ELSEIF (UNIX)
    MESSAGE(STATUS "Platform is UNIX")
    add_definitions(-DUNIX)
ENDIF ()

find_package(PkgConfig REQUIRED)

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED
        )

pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
        libavdevice
        libavfilter
        libavformat
        libavcodec
        libswresample
        libswscale
        libavutil
        )

add_library(lib
        src/ScreenRecorder.cpp
        src/SettingsConf.cpp
        src/GetAudioDevices.cpp
        src/MemoryCheck.cpp
        src/AreaSelector.cpp
        include/ScreenRecorder.h
        include/SettingsConf.h
        include/GetAudioDevices.h
        include/MemoryCheck.h
        include/AreaSelector.h
        include/Common.h)

target_include_directories(lib PUBLIC include/)

target_link_libraries(${PROJECT_NAME}
        PkgConfig::LIBAV
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        )