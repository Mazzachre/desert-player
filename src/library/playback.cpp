#include "playback.h"
#include <QDateTime>
#include "playlists.h"
#include "../app/config.h"

QPointer<dp::library::Playback> dp::library::Playback::m_instance = nullptr;

dp::library::Playback::Playback(QObject* parent)
	:QObject(parent) {
}

dp::library::Playback::~Playback() {}

void dp::library::Playback::init(QObject* parent) {
	if (!m_instance) m_instance = new Playback(parent);
}

dp::library::Playback* dp::library::Playback::instance() {
	if (!m_instance) qFatal("Playback used before initialization");
	return m_instance;
}

void dp::library::Playback::startPlayback(const File& file) {
	m_file = file;
    m_started = QDateTime::currentDateTime().toSecsSinceEpoch();
	m_accumulated = 0;
	m_position = 0;
}

void dp::library::Playback::stopPlayback() {
	if (m_file.isValid() && m_accumulated > dp::app::Config::instance()->watchedBeforeSave()) {
		QVariantList l_playbacks = m_file.playbackData;
		QVariantMap l_playback;
		l_playback["started"] = m_started;
		l_playback["position"] = m_position;
		l_playback["accumulated"] = m_accumulated;
		l_playbacks << l_playback;
		Q_EMIT playbackFinished(m_file.path, l_playbacks, Playlists::instance()->getSelected());
		m_file = File();
	}
}

void dp::library::Playback::videoTrackSelected(const QVariant& trackId) {
	if (m_file.isValid() && trackId.isValid()) {
		QVariantMap l_tracks = m_file.tracks;
		l_tracks["videoTrack"] = trackId;
		Q_EMIT tracksUpdated(m_file.path, l_tracks, Playlists::instance()->getSelected());
	}
}

void dp::library::Playback::audioTrackSelected(const QVariant& trackId) {
	if (m_file.isValid() && trackId.isValid()) {
		QVariantMap l_tracks = m_file.tracks;
		l_tracks["audioTrack"] = trackId;
		Q_EMIT tracksUpdated(m_file.path, l_tracks, Playlists::instance()->getSelected());
	}
}

void dp::library::Playback::subtitleTrackSelected(const QVariant& trackId) {
	if (m_file.isValid() && trackId.isValid()) {
		QVariantMap l_tracks = m_file.tracks;
		l_tracks["subtitleTrack"] = trackId;
		Q_EMIT tracksUpdated(m_file.path, l_tracks, Playlists::instance()->getSelected());
	}
}

void dp::library::Playback::subtitleTrackRemoved(const QVariant& trackId) {
	if (m_file.isValid() && trackId.isValid()) {
		QVariantMap l_tracks = m_file.tracks;
		l_tracks.remove("subtitleTrack");
		Q_EMIT tracksUpdated(m_file.path, l_tracks, Playlists::instance()->getSelected());
		qDebug() << "Delete subtitle file?" << trackId;
	}
}

void dp::library::Playback::volumeChanged(uint volume) {
	if (m_file.isValid() && volume > 0) {
		QVariantMap l_tracks = m_file.tracks;
		l_tracks["volume"] = volume;
		Q_EMIT tracksUpdated(m_file.path, l_tracks, Playlists::instance()->getSelected());
	}
}

void dp::library::Playback::positionChanged(qulonglong position) {
	m_accumulated++;
	m_position = position;
}
