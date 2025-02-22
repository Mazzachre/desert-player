cmake_minimum_required(VERSION 3.16)
project(desert-player)

set(QT_MIN_VERSION "5.12.0")

find_package(ECM "5.68.0" REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Quick OpenGL Test Gui QuickControls2 Widgets Sql Concurrent Network X11Extras)
find_package(X11 REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

add_executable(desert-player)

target_sources(desert-player
	PRIVATE
	src/main.cpp
	src/app/app.cpp
	src/app/config.cpp
	src/app/logger.cpp
	src/data/base.cpp
	src/data/data.cpp
	src/data/types.cpp
	src/files/identify.cpp
	src/files/meta.cpp
	src/library/playlists.cpp
	src/library/file_list.cpp
	src/library/playlist_controller.cpp
	src/library/files_worker.cpp
	src/library/playback.cpp
	src/player/mpv_controller.cpp
	src/player/mpv_player.cpp
	src/player/mpv_renderer.cpp
	src/player/audio_list.cpp
	src/player/video_list.cpp
	src/player/subtitle_list.cpp
	src/ui/window_controller.cpp
	src/ui/keep_alive.cpp
	src/resources.qrc
)

target_link_libraries(desert-player
	Qt5::Quick
	Qt5::Qml
	Qt5::Gui
	Qt5::QuickControls2
	Qt5::Widgets
	Qt5::Sql
	Qt5::Network
	Qt5::X11Extras
	X11
	mpv
	avformat
	avcodec
	avutil
)

# Install binary
install(TARGETS desert-player RUNTIME DESTINATION bin)
message("Installing target: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/desert-player to ${CMAKE_INSTALL_PREFIX}/bin")

# Install the .desktop file
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/debian/desert-player.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)

# Install the icons
set(ICON_SIZES 16x16 32x32 48x48 64x64 128x128 256x256)
foreach(SIZE ${ICON_SIZES})
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/debian/icons/${SIZE}/ DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${SIZE}/apps
            FILES_MATCHING PATTERN "*.png")
endforeach()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
