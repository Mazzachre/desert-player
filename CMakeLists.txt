cmake_minimum_required(VERSION 3.16)
project(desert_player)

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

add_executable(desert_player)

target_sources(desert_player
	PRIVATE
	src/main.cpp
	src/app/app.cpp
	src/app/config.cpp
	src/data/data.cpp
	src/data/types.cpp
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

target_link_libraries(desert_player
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

install(TARGETS desert_player RUNTIME DESTINATION bin)
message("Installing target: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/desert_player to ${CMAKE_INSTALL_PREFIX}/bin")

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
