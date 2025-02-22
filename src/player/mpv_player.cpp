#include <QThread>

#include "mpv_player.h"
#include "mpv_renderer.h"
#include "../app/config.h"

inline bool test_type(const QVariant &v, QMetaType::Type t) {
	return static_cast<int>(v.type()) == static_cast<int>(t);
}

dp::player::MpvPlayer::MpvPlayer(QQuickItem *parent)
	: QQuickFramebufferObject(parent)
	, m_controllerThread{new QThread}
	, m_mpvController{new MpvController(dp::app::Config::instance()->debug())} {
	m_controllerThread->start();
	m_mpvController->moveToThread(m_controllerThread);
	m_mpv = m_mpvController->mpv();

	m_mpvController->observeProperty("time-pos", MPV_FORMAT_INT64);
	m_mpvController->observeProperty("duration", MPV_FORMAT_INT64);
	m_mpvController->observeProperty("track-list", MPV_FORMAT_NODE);
	m_mpvController->observeProperty("pause", MPV_FORMAT_FLAG);
	m_mpvController->observeProperty("volume", MPV_FORMAT_DOUBLE);
	m_mpvController->observeProperty("mute", MPV_FORMAT_FLAG);
	m_mpvController->observeProperty("sub-visibility", MPV_FORMAT_FLAG);
	m_mpvController->observeProperty("vid", MPV_FORMAT_INT64);
	m_mpvController->observeProperty("aid", MPV_FORMAT_INT64);
	m_mpvController->observeProperty("sid", MPV_FORMAT_INT64);

	connect(m_mpvController, &MpvController::propertyChanged, this, &MpvPlayer::handlePropertyChange);
	connect(m_mpvController, &MpvController::tracksLoaded, this, &MpvPlayer::handleTracksLoaded);
	connect(m_mpvController, &MpvController::endFile, this, &MpvPlayer::handleFileEnd);
	connect(m_mpvController, &MpvController::endError, this, &MpvPlayer::handleFileError);

	// clang-format off
	connect(m_controllerThread, &QThread::finished, m_mpvController, &MpvController::deleteLater);
	// clang-format on
}

dp::player::MpvPlayer::~MpvPlayer() {
	if (m_mpv_gl) {
		mpv_render_context_free(m_mpv_gl);
	}
	mpv_set_wakeup_callback(m_mpv, nullptr, nullptr);

	m_controllerThread->quit();
	m_controllerThread->wait();
	m_controllerThread->deleteLater();
	mpv_terminate_destroy(m_mpv);
}

void dp::player::MpvPlayer::handlePropertyChange(const QString &property, const QVariant &value) {
	if (property.compare("pause") == 0) Q_EMIT pausedChanged(value.toBool());
	else if (property.compare("time-pos") == 0 && value.isValid()) Q_EMIT positionChanged(value.toULongLong());//TODO Here we can do skippety skip!
	else if (property.compare("duration") == 0 && value.isValid()) Q_EMIT durationChanged(value.toULongLong());
	else if (property.compare("mute") == 0) Q_EMIT mutedChanged(value.toBool());
	else if (property.compare("sub-visibility") == 0) Q_EMIT subtitlesVisibleChanged(value.toBool());
	else if (property.compare("volume") == 0) Q_EMIT volumeChanged(value.isNull() ? 0 : value.toUInt());
	else if (property.compare("vid") == 0) Q_EMIT videoTrackChanged(value);
	else if (property.compare("aid") == 0) Q_EMIT audioTrackChanged(value);
	else if (property.compare("sid") == 0) {
		if (m_subtitle_files.contains(value.toUInt())) Q_EMIT subtitleTrackChanged(m_subtitle_files.value(value.toUInt()));
		else Q_EMIT subtitleTrackChanged(value.toUInt());
	} else if (property.compare("track-list") == 0) {
		QList<QVariant> videoTracks;
		QList<QVariant> audioTracks;
		QList<QVariant> subtitleTracks;
		m_subtitle_files.clear();
		
		Q_FOREACH (const QVariant& track, value.toList()) {
			QString type = track.toMap()["type"].toString();
			if (type.compare("video") == 0) videoTracks << track;
			if (type.compare("audio") == 0) audioTracks << track;
			if (type.compare("sub") == 0) {
				subtitleTracks << track;
				if (track.toMap().value("external-filename").isValid())
					m_subtitle_files[track.toMap()["id"].toUInt()] = track.toMap().value("external-filename").toString();
			}
		}

		Q_EMIT videoTracksLoaded(videoTracks);
		Q_EMIT audioTracksLoaded(audioTracks);
		Q_EMIT subtitleTracksLoaded(subtitleTracks);
	} else qDebug() << "Unknown property change: " << property << " - " << value;
}

void dp::player::MpvPlayer::handleFileEnd() const {
	Q_EMIT movieStopped();
}

void dp::player::MpvPlayer::handleFileError(const QString& reason) const {
	if (!reason.isEmpty()) {
		Q_EMIT error(reason);
	}
	
	Q_EMIT movieStopped();
}

void dp::player::MpvPlayer::playFile(const File& file) {
	m_movie = file.tracks;
	m_movie["movie"] = file.path;
	QVariant result = m_mpvController->command(QVariant{QStringList{"loadfile", file.path}});
	if (result.canConvert<MpvErrorType>() && result.value<MpvErrorType>().isValid()) {
		Q_EMIT error(result.value<MpvErrorType>().toString());
	}	
}

void dp::player::MpvPlayer::stopMovie() const {
	QVariant result = m_mpvController->command(QVariant{QStringList{"playlist-remove", "current"}});
	if (result.canConvert<MpvErrorType>() && result.value<MpvErrorType>().isValid()) {
		Q_EMIT error(result.value<MpvErrorType>().toString());
	}
	Q_EMIT movieCancelled();
}

void dp::player::MpvPlayer::handleTracksLoaded() const {
	qDebug() << "Tracks loaded: " << m_movie;

	changeSubtitleTrack(m_movie.value("subtitleTrack"));
	changeVideoTrack(m_movie.value("videoTrack"));
	changeAudioTrack(m_movie.value("audioTrack"));
	if (m_movie.contains("volume")) {
		changeVolume(m_movie.value("volume").toUInt());
	} else {
		changeVolume(100U);
	}
	QVariant result = m_mpvController->setProperty("pause", false);	
	if (result.canConvert<MpvErrorType>() && result.value<MpvErrorType>().isValid()) {
		Q_EMIT error(result.value<MpvErrorType>().toString());
	}
	Q_EMIT movieStarted(m_movie.value("movie").toString());
}

QQuickFramebufferObject::Renderer *dp::player::MpvPlayer::createRenderer() const {
    return new MpvRenderer(const_cast<MpvPlayer *>(this));
}

void dp::player::MpvPlayer::changePosition(const QVariant& position) const {
	if (test_type(position, QMetaType::ULongLong)) {
		QVariant result = m_mpvController->setProperty("time-pos", position.toULongLong());
		if (result.canConvert<MpvErrorType>() && result.value<MpvErrorType>().isValid()) {
			Q_EMIT error(result.value<MpvErrorType>().toString());
		}
	}
}

void dp::player::MpvPlayer::seekPosition(qlonglong amount) const {
	QVariant result = m_mpvController->command(QVariant{QVariantList{"seek", amount}});	
	if (result.canConvert<MpvErrorType>() && result.value<MpvErrorType>().isValid()) {
		Q_EMIT error(result.value<MpvErrorType>().toString());
	}
}

void dp::player::MpvPlayer::togglePause() const {
	QVariant result = m_mpvController->command(QVariant{QStringList{"cycle", "pause"}});
	if (result.canConvert<MpvErrorType>() && result.value<MpvErrorType>().isValid()) {
		Q_EMIT error(result.value<MpvErrorType>().toString());
	}
}

void dp::player::MpvPlayer::setPause(bool pause) const {
	QVariant result = m_mpvController->setProperty("pause", pause);
	if (result.canConvert<MpvErrorType>() && result.value<MpvErrorType>().isValid()) {
		Q_EMIT error(result.value<MpvErrorType>().toString());
	}	
}

void dp::player::MpvPlayer::changeVolume(const QVariant& volume) const {
	if (test_type(volume, QMetaType::UInt)) {
		QVariant result = m_mpvController->setProperty("volume", volume.toUInt());
		if (result.canConvert<MpvErrorType>() && result.value<MpvErrorType>().isValid()) {
			Q_EMIT error(result.value<MpvErrorType>().toString());
		}
	}
}

void dp::player::MpvPlayer::toggleMute() const {
	QVariant result = m_mpvController->command(QVariant{QStringList{"cycle", "mute"}});
	if (result.canConvert<MpvErrorType>() && result.value<MpvErrorType>().isValid()) {
		Q_EMIT error(result.value<MpvErrorType>().toString());
	}
}

void dp::player::MpvPlayer::changeVideoTrack(const QVariant& trackId) const {
	if (trackId.canConvert<int>()) {
		QVariant result = m_mpvController->setProperty("vid", trackId.toUInt());
		if (result.canConvert<MpvErrorType>() && result.value<MpvErrorType>().isValid()) {
			Q_EMIT error(result.value<MpvErrorType>().toString());
		}
	}
}

void dp::player::MpvPlayer::changeAudioTrack(const QVariant& trackId) const {
	if (trackId.canConvert<int>()) {
		QVariant result = m_mpvController->setProperty("aid", trackId.toUInt());
		if (result.canConvert<MpvErrorType>() && result.value<MpvErrorType>().isValid()) {
			Q_EMIT error(result.value<MpvErrorType>().toString());
		}
	}
}

void dp::player::MpvPlayer::changeSubtitleTrack(const QVariant& subtitle) const {
	QVariant result;
	if (test_type(subtitle, QMetaType::QString) || test_type(subtitle, QMetaType::QUrl)) {
		result = m_mpvController->command(QVariant{QStringList{"sub-add", subtitle.toString()}});		
	} else if (subtitle.canConvert<int>()) {
		result = m_mpvController->setProperty("sid", subtitle.toUInt());
	}
	if (result.canConvert<MpvErrorType>() && result.value<MpvErrorType>().isValid()) {
		Q_EMIT error(result.value<MpvErrorType>().toString());
	}
	result = m_mpvController->command(QVariant{QStringList{"sub-visibility", "yes"}});
	if (result.canConvert<MpvErrorType>() && result.value<MpvErrorType>().isValid()) {
		Q_EMIT error(result.value<MpvErrorType>().toString());
	}
}

uint findSubtitleId(const QMap<uint, QString>& map, const QString& value) {
	for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
		if (it.value() == value) {
			return it.key();
		}
	}

	return 0;
}

void dp::player::MpvPlayer::removeSubtitleTrack(const QVariant& subtitle) const {
	uint id = findSubtitleId(m_subtitle_files, subtitle.toString());
	if (id > 0) {
		QVariant result = m_mpvController->command(QVariant{QStringList{"sub-remove", QString::number(id)}});
		Q_EMIT subtitleTrackRemoved(m_subtitle_files[id]);
		if (result.canConvert<MpvErrorType>() && result.value<MpvErrorType>().isValid()) {
			Q_EMIT error(result.value<MpvErrorType>().toString());
		}
	} else {
		Q_EMIT error("Cannot remove unknown subtitle " + subtitle.toString());
	}
}

void dp::player::MpvPlayer::toggleSubtitles() const {
	QVariant result = m_mpvController->command(QVariant{QStringList{"cycle", "sub-visibility"}});
	if (result.canConvert<MpvErrorType>() && result.value<MpvErrorType>().isValid()) {
		Q_EMIT error(result.value<MpvErrorType>().toString());
	}
}
