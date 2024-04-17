#include "file_list.h"
#include <QUrl>
#include <QTime>
#include <QDateTime>
#include <QTimeZone>
#include "../app/config.h"
#include "./playlists.h"

QPointer<dp::library::FileList> dp::library::FileList::m_instance = nullptr;

dp::library::FileList::FileList(QObject* parent)
	: QAbstractListModel(parent) {}

void dp::library::FileList::init(QObject* parent) {
	if (!m_instance) m_instance = new FileList(parent);
}

dp::library::FileList* dp::library::FileList::instance() {
	if (!m_instance) qFatal("File list accessed before initalization");
	return m_instance;
}

QHash<int, QByteArray> dp::library::FileList::roleNames() const {
	QHash<int, QByteArray> roles;
	roles[IdRole] = "id";
	roles[PathRole] = "path";
	roles[TitleRole] = "title";
	roles[DurationRole] = "duration";
	roles[HasSubtitleRole] = "hasSubtitle";
	roles[HasIntSubtitleRole] = "internalSubtitle";
	roles[SubtitleTrackRole] = "subtitleTrack";
	roles[WasStartedRole] = "wasStarted";
	roles[StartedRecentlyRole] = "startedRecently";
	roles[WasPlayedRole] = "wasPlayed";
	roles[PlayedListRole] = "playedList";
	roles[SelectedRole] = "selected";
	return roles;
}

int dp::library::FileList::rowCount(const QModelIndex&) const {
	return m_backing.size();	
}

bool wasStarted(const File& file) {
	return !file.playbackData.isEmpty();
}

bool wasStartedRecently(const File& file) {
	if (!wasStarted(file)) return false;
	qlonglong l_now = QDateTime::currentDateTime().toSecsSinceEpoch();
	QVariantMap l_continueData = file.playbackData.isEmpty() ? QVariantMap() : file.playbackData.last().toMap();
	return ((l_now - l_continueData["started"].toLongLong()) < dp::app::Config::instance()->showContinueBefore());
}

bool wasPlayed(const File& file) {
	if (!wasStarted(file)) return false;
	QVariantMap l_continueData = file.playbackData.isEmpty() ? QVariantMap() : file.playbackData.last().toMap();
	uint played = file.mediaMeta["duration"].isValid() ? (100* l_continueData["position"].toUInt()) / file.mediaMeta["duration"].toUInt() : 50;
	return played >= dp::app::Config::instance()->ignoreContinueAfter();
}

QString formatPlayed(qulonglong played) {
	unsigned int l_hours = played / 3600;
	unsigned int l_minutes = (played % 3600) / 60;
	unsigned int l_seconds = played % 60;

	QString l_result;
	if (l_hours > 0) l_result += QString("%1:").arg(l_hours, 2, 10, QChar('0'));
	l_result += QString("%1:%2").arg(l_minutes, 2, 10, QChar('0')).arg(l_seconds, 2, 10, QChar('0'));

	return l_result;
}

QString playedList(const File& file) {
	if (file.playbackData.isEmpty()) return "";

	QString l_played = QStringLiteral("<table cellpadding=\"4\" cellspacing=\"2\"><tr bgcolor=\"lightsteelblue\"><th>Date</th><th>Played</th></tr>");
	for (auto& item : file.playbackData) {
		QVariantMap l_item = item.toMap();
		QString l_started = QDateTime::fromSecsSinceEpoch(l_item["started"].toLongLong(), QTimeZone::systemTimeZone()).toString(dp::app::Config::instance()->dateFormat() + " " + dp::app::Config::instance()->timeFormat());
		l_played += QStringLiteral("<tr><td>") + l_started + QStringLiteral("</td><td>") + formatPlayed(l_item["position"].toULongLong()) + QStringLiteral("</td></tr>");
	}
	l_played += QStringLiteral("</table>");

	return l_played;
}

QString subtitleTrack(const File& file) {
	QString l_subtitle = QStringLiteral("");

	if (file.tracks.contains("subtitleTrack")) {
		if (file.tracks.value("subtitleTrack").type() == QVariant::String) {
			l_subtitle = QStringLiteral("External ") + QUrl(file.tracks.value("subtitleTrack").toString()).fileName();
		} else {
			l_subtitle = QStringLiteral("Internal ") + file.getSubtitleLabel(file.tracks.value("subtitleTrack").toUInt());
		}
	} else if (file.hasInternalSubtitleStream()) {
		l_subtitle = QStringLiteral("Unselected internal");
	}
	
	return l_subtitle;
}

QVariant dp::library::FileList::data(const QModelIndex &index, int role) const {
	QVariant l_result;
	if (index.isValid()) {
		File l_file = m_backing[Playlists::instance()->getFiles()[index.row()]];
		switch (role) {
			case IdRole:
				l_result = l_file.id;
				break;
			case PathRole:
				l_result = l_file.path;
				break;
			case TitleRole:
				l_result = l_file.mediaMeta.value("title");
				break;
			case DurationRole:
				l_result = l_file.mediaMeta.value("duration").isValid() ? QTime(0,0).addSecs(l_file.mediaMeta.value("duration").toUInt()).toString("H:mm:ss") : "??:??";
				break;
			case HasSubtitleRole:
				l_result = l_file.tracks.contains("subtitleTrack");
				break;
			case HasIntSubtitleRole:
				l_result = l_file.hasInternalSubtitleStream();
				break;
			case SubtitleTrackRole:
				l_result = subtitleTrack(l_file);
				break;
			case WasStartedRole:
				l_result = wasStarted(l_file);
				break;
			case StartedRecentlyRole:
				l_result = wasStartedRecently(l_file);
				break;
			case WasPlayedRole:
				l_result = wasPlayed(l_file);
				break;
			case PlayedListRole:
				l_result = playedList(l_file);
				break;
			case SelectedRole:
				l_result = l_file.id == m_selected;
				break;
		}
	}
	return l_result;
}

void dp::library::FileList::setFileList(const QVector<File>& files, qulonglong playlistId) {
	beginResetModel();
	m_backing.clear();
	m_paths.clear();

	for (const File& file : files) {
		m_backing[file.id] = file;
		m_paths[file.path] = file.id;
	}

	if (m_backing.isEmpty()) {
		m_selected = 0;
	} else {
		if (playlistId != m_playlistId) {
			m_playlistId = playlistId;
			m_selected = Playlists::instance()->getFiles()[0];
		} else if(!m_backing.contains(m_selected)) {
			m_selected = Playlists::instance()->getFiles()[0];			
		}
	}

	Q_EMIT playableUpdated();
	endResetModel();
}

void dp::library::FileList::selectFile(const QString& path) {
	auto l_prev = index(Playlists::instance()->getFiles().indexOf(m_selected), 0);
	auto l_next = index(Playlists::instance()->getFiles().indexOf(m_paths[path]), 0);
	
	m_selected = m_paths[path];
	Q_EMIT fileSelected(path);
	Q_EMIT dataChanged(l_prev, l_prev, {SelectedRole});
	Q_EMIT dataChanged(l_next, l_next, {SelectedRole});
}

void dp::library::FileList::fileUpdated(const File& file) {
	auto l_index = index(Playlists::instance()->getFiles().indexOf(file.id), 0);
	m_backing[file.id] = file;
	Q_EMIT dataChanged(l_index, l_index, {
		TitleRole,
		HasSubtitleRole,
		HasIntSubtitleRole,
		SubtitleTrackRole,
		WasStartedRole,
		StartedRecentlyRole,
		WasPlayedRole,
		PlayedListRole
	});
}

void dp::library::FileList::startPlaying() {
	if (m_backing.size() > 0 && m_backing.contains(m_selected)) {
		Q_EMIT playFile(m_backing[m_selected]);
	}
}

void dp::library::FileList::playPrev() {
	if (hasPrev()) {
		int l_index = Playlists::instance()->getFiles().indexOf(m_selected) - 1;
		Q_EMIT playFile(m_backing[Playlists::instance()->getFiles()[l_index]]);
	} else {
		Q_EMIT playlistFinished();
	}
}

void dp::library::FileList::playNext() {
	if (hasNext()) {
		int l_index = Playlists::instance()->getFiles().indexOf(m_selected) + 1;
		Q_EMIT playFile(m_backing[Playlists::instance()->getFiles()[l_index]]);
	} else {
		Q_EMIT playlistFinished();
	}
}

bool dp::library::FileList::hasPrev() {
	return m_backing.size() != 0 && Playlists::instance()->getFiles().first() != m_selected;
}

bool dp::library::FileList::hasNext() {
	return m_backing.size() != 0 && Playlists::instance()->getFiles().last() != m_selected;
}

bool dp::library::FileList::getPlayable() {
	return m_backing.size() > 0;
}

void dp::library::FileList::updateFileTitle(qulonglong id, const QString& title) {
	File l_file = m_backing.value(id);
	if (l_file.isValid()) {
		QVariantMap l_mediaMeta = l_file.mediaMeta;
		l_mediaMeta["title"] = title;
		Q_EMIT updateMediaMeta(id, l_mediaMeta);
	}
}

void dp::library::FileList::updateSubtitle(qulonglong id, const QString& path) {
	File l_file = m_backing.value(id);
	if (l_file.isValid()) {
		QVariantMap l_tracks = l_file.tracks;
		l_tracks["subtitleTrack"] = path;
		Q_EMIT updateTracks(id, l_tracks);		
	}
}
