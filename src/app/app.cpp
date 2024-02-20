#include "app.h"

#include <QDesktopWidget>
#include <QCommandLineParser>
#include "config.h"
#include "../data/data.h"
#include "../library/playlists.h"
#include "../library/file_list.h"
#include "../library/playlist_controller.h"
#include "../library/playback.h"
#include "../player/mpv_player.h"
#include "../player/audio_list.h"
#include "../player/video_list.h"
#include "../player/subtitle_list.h"
#include "../ui/window_controller.h"

#include <QDebug>

dp::app::App::App(int &argc, char **argv)
	: QApplication(argc, argv)
	, m_engine{new QQmlApplicationEngine}
	, m_keepAlive{new dp::ui::KeepAlive(this)} {
	QCommandLineParser parser;
	parser.addOptions({
		{{"V", "Verbose"}, "Verbose terminal logging"}
	});
	parser.process(*this);

	Config::init(this);
	dp::data::Data::init(this);
	dp::library::Playlists::init(this);
	dp::library::FileList::init(this);
	dp::library::PlaylistController::init(this);
	dp::library::Playback::init(this);
	dp::player::AudioList::init(this);
	dp::player::VideoList::init(this);
	dp::player::SubtitleList::init(this);
	dp::ui::WindowController::init(this);

	registerQmlTypes();
	m_engine->load(QUrl(QStringLiteral("qrc:/main.qml")));

	QDesktopWidget* desktopWidget = desktop();
	QQuickWindow *window = qobject_cast<QQuickWindow*>(m_engine->rootObjects().first());
	if (!window) qFatal("No root window found");
	dp::ui::WindowController::instance()->init(window, desktopWidget->screenGeometry(desktopWidget->primaryScreen()));
}

dp::app::App::~App() {
	m_engine->deleteLater();
}

int dp::app::App::run() {
	if (m_engine->rootObjects().isEmpty()) return -1;
	connectSignals();
	dp::data::Data::instance()->startup();
	return exec();
}

void dp::app::App::connectSignals() {
	QObject *rootObject = m_engine->rootObjects().first();
	dp::player::MpvPlayer* player = rootObject->findChild<dp::player::MpvPlayer*>("player");
	if (!player) qFatal("Unable to find player");

	connect(player, &dp::player::MpvPlayer::positionChanged, m_keepAlive, &dp::ui::KeepAlive::moviePlaying);

	//Playlist to data
	connect(dp::library::Playlists::instance(), &dp::library::Playlists::playlistSelected, dp::data::Data::instance(), &dp::data::Data::playlistSelected, Qt::QueuedConnection);
	connect(dp::data::Data::instance(), &dp::data::Data::playlistsUpdated, dp::library::Playlists::instance(), &dp::library::Playlists::setPlaylists);
	connect(dp::data::Data::instance(), &dp::data::Data::fileListUpdated, dp::library::FileList::instance(), &dp::library::FileList::setFileList);
	connect(dp::library::PlaylistController::instance(), &dp::library::PlaylistController::createPlaylist, dp::data::Data::instance(), &dp::data::Data::addPlaylist, Qt::QueuedConnection);
	connect(dp::library::PlaylistController::instance(), &dp::library::PlaylistController::deletePlaylist, dp::data::Data::instance(), &dp::data::Data::removePlaylist, Qt::QueuedConnection);
	connect(dp::library::PlaylistController::instance(), &dp::library::PlaylistController::updatePlaylistFiles, dp::data::Data::instance(), &dp::data::Data::updatePlaylistFiles, Qt::QueuedConnection);
	connect(dp::library::PlaylistController::instance(), &dp::library::PlaylistController::updatePlaylistName, dp::data::Data::instance(), &dp::data::Data::updatePlaylistName, Qt::QueuedConnection);

	//UI to player
	connect(dp::ui::WindowController::instance(), &dp::ui::WindowController::playerTogglePause, player, &dp::player::MpvPlayer::togglePause);
	connect(dp::ui::WindowController::instance(), &dp::ui::WindowController::playerToggleSubtitles, player, &dp::player::MpvPlayer::toggleSubtitles);
	connect(dp::ui::WindowController::instance(), &dp::ui::WindowController::playerSeek, player, &dp::player::MpvPlayer::seekPosition);

	//Player to video list
	connect(player, &dp::player::MpvPlayer::videoTracksLoaded, dp::player::VideoList::instance(), &dp::player::VideoList::setTracks);
	connect(player, &dp::player::MpvPlayer::videoTrackChanged, dp::player::VideoList::instance(), &dp::player::VideoList::setSelected);
	connect(player, &dp::player::MpvPlayer::pausedChanged, dp::player::VideoList::instance(), &dp::player::VideoList::setPaused);
	connect(player, &dp::player::MpvPlayer::positionChanged, dp::player::VideoList::instance(), &dp::player::VideoList::setPosition);
	connect(player, &dp::player::MpvPlayer::durationChanged, dp::player::VideoList::instance(), &dp::player::VideoList::setDuration);
	connect(dp::player::VideoList::instance(), &dp::player::VideoList::positionChange, player, &dp::player::MpvPlayer::changePosition);

	//Player to audio list
	connect(player, &dp::player::MpvPlayer::audioTracksLoaded, dp::player::AudioList::instance(), &dp::player::AudioList::setTracks);
	connect(player, &dp::player::MpvPlayer::audioTrackChanged, dp::player::AudioList::instance(), &dp::player::AudioList::setSelected);
	connect(player, &dp::player::MpvPlayer::mutedChanged, dp::player::AudioList::instance(), &dp::player::AudioList::setMuted);
	connect(player, &dp::player::MpvPlayer::volumeChanged, dp::player::AudioList::instance(), &dp::player::AudioList::setVolume);
	connect(dp::player::AudioList::instance(), &dp::player::AudioList::volumeChange, player, &dp::player::MpvPlayer::changeVolume);

	//Player to subtitle list
	connect(player, &dp::player::MpvPlayer::subtitleTracksLoaded, dp::player::SubtitleList::instance(), &dp::player::SubtitleList::setTracks);
	connect(player, &dp::player::MpvPlayer::subtitleTrackChanged, dp::player::SubtitleList::instance(), &dp::player::SubtitleList::setSelected);
	connect(player, &dp::player::MpvPlayer::movieStarted, dp::player::SubtitleList::instance(), &dp::player::SubtitleList::movieStarted);
	connect(dp::player::SubtitleList::instance(), &dp::player::SubtitleList::removeSubtitleTrack, player, &dp::player::MpvPlayer::removeSubtitleTrack);

	//Player to playback collecter
	connect(player, &dp::player::MpvPlayer::subtitleTrackChanged, dp::library::Playback::instance(), &dp::library::Playback::subtitleTrackSelected);
	connect(player, &dp::player::MpvPlayer::audioTrackChanged, dp::library::Playback::instance(), &dp::library::Playback::audioTrackSelected);
	connect(player, &dp::player::MpvPlayer::volumeChanged, dp::library::Playback::instance(), &dp::library::Playback::volumeChanged);
	connect(player, &dp::player::MpvPlayer::videoTrackChanged, dp::library::Playback::instance(), &dp::library::Playback::videoTrackSelected);
	connect(player, &dp::player::MpvPlayer::positionChanged, dp::library::Playback::instance(), &dp::library::Playback::positionChanged);
	connect(player, &dp::player::MpvPlayer::movieCancelled, dp::library::Playback::instance(), &dp::library::Playback::stopPlayback);
	connect(player, &dp::player::MpvPlayer::movieStopped, dp::library::Playback::instance(), &dp::library::Playback::stopPlayback);

	//Playlist connections
	connect(dp::library::FileList::instance(), &dp::library::FileList::playFile, dp::ui::WindowController::instance(), &dp::ui::WindowController::startPlaying);
	connect(dp::library::FileList::instance(), &dp::library::FileList::playFile, player, &dp::player::MpvPlayer::playFile);
	connect(dp::library::FileList::instance(), &dp::library::FileList::playFile, dp::library::Playback::instance(), &dp::library::Playback::startPlayback);
	connect(dp::library::FileList::instance(), &dp::library::FileList::playFile, dp::player::VideoList::instance(), &dp::player::VideoList::startPlayback);
	connect(player, &dp::player::MpvPlayer::movieCancelled, dp::ui::WindowController::instance(), &dp::ui::WindowController::stopPlaying);
	connect(player, &dp::player::MpvPlayer::movieStopped, dp::library::FileList::instance(), &dp::library::FileList::playNext);
	connect(player, &dp::player::MpvPlayer::movieStarted, dp::library::FileList::instance(), &dp::library::FileList::selectFile);
	connect(dp::library::FileList::instance(), &dp::library::FileList::playlistFinished, dp::ui::WindowController::instance(), &dp::ui::WindowController::stopPlaying);

	//Playback to data connections
	connect(dp::library::Playback::instance(), &dp::library::Playback::tracksUpdated, dp::data::Data::instance(), &dp::data::Data::updateTracks);
	connect(dp::library::Playback::instance(), &dp::library::Playback::playbackFinished, dp::data::Data::instance(), &dp::data::Data::updatePlaybackData);

	//handle errors - should be a display somewhere
	connect(dp::data::Data::instance(), &dp::data::Data::error, this, &App::handleError);
	connect(dp::library::PlaylistController::instance(), &dp::library::PlaylistController::error, this, &App::handleError);
	connect(player, &dp::player::MpvPlayer::error, this, &App::handleError);
}

void dp::app::App::registerQmlTypes() {
	qRegisterMetaType<File>();
	qRegisterMetaType<QList<File>>();
	
	qmlRegisterType<dp::player::MpvPlayer>("desert", 1, 0, "MpvPlayer");

	qmlRegisterSingletonType<dp::ui::WindowController>("desert", 1, 0, "WindowController",
		[](QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> QObject* {
			Q_UNUSED(qmlEngine);
			Q_UNUSED(jsEngine);
			return dp::ui::WindowController::instance();
		}
	);

	qmlRegisterSingletonType<dp::library::Playlists>("desert", 1, 0, "Playlists",
		[](QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> QObject* {
			Q_UNUSED(qmlEngine);
			Q_UNUSED(jsEngine);
			return dp::library::Playlists::instance();
		}
	);

	qmlRegisterSingletonType<dp::library::FileList>("desert", 1, 0, "FileList",
		[](QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> QObject* {
			Q_UNUSED(qmlEngine);
			Q_UNUSED(jsEngine);
			return dp::library::FileList::instance();
		}
	);

	qmlRegisterSingletonType<dp::library::PlaylistController>("desert", 1, 0, "PlaylistController",
		[](QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> QObject* {
			Q_UNUSED(qmlEngine);
			Q_UNUSED(jsEngine);
			return dp::library::PlaylistController::instance();
		}
	);

	qmlRegisterSingletonType<dp::player::AudioList>("desert", 1, 0, "Audio",
		[](QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> QObject* {
			Q_UNUSED(qmlEngine);
			Q_UNUSED(jsEngine);
			return dp::player::AudioList::instance();
		}
	);

	qmlRegisterSingletonType<dp::player::VideoList>("desert", 1, 0, "Video",
		[](QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> QObject* {
			Q_UNUSED(qmlEngine);
			Q_UNUSED(jsEngine);
			return dp::player::VideoList::instance();
		}
	);

	qmlRegisterSingletonType<dp::player::SubtitleList>("desert", 1, 0, "Subtitle",
		[](QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> QObject* {
			Q_UNUSED(qmlEngine);
			Q_UNUSED(jsEngine);
			return dp::player::SubtitleList::instance();
		}
	);
}

void dp::app::App::handleError(const QString& error) {
	QObject* senderObject = sender();
	if (senderObject) {
		const QMetaObject* metaObject = senderObject->metaObject();
		const char* className = metaObject->className();

		qDebug() << "Error!" << className << error;
	} else {
		qDebug() << "Error! Unknown" << error;
	}
}
