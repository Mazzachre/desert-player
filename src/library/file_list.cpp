#include "file_list.h"
#include <QTime>
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
	roles[WasPlayedRole] = "wasPlayed";
	roles[SelectedRole] = "selected";
	return roles;
}

int dp::library::FileList::rowCount(const QModelIndex&) const {
	return m_backing.size();	
}

bool wasPlayed(const File& file) {
	qlonglong l_now = QDateTime::currentDateTime().toSecsSinceEpoch();
	QVariantMap l_continueData = file.playbackData.isEmpty() ? QVariantMap() : file.playbackData.last().toMap();
	return ((l_now - l_continueData["started"].toLongLong()) < dp::app::Config::instance()->showContinueBefore());
}

QString playedList(const File& file) {
	qDebug() << file.playbackData;
	
	//loop to make html string...
	
	return "";
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
				l_result = l_file.hasSubtitleStream(dp::app::Config::instance()->language(dp::app::Config::LanguageCodes::ISO_639_2)) || l_file.tracks.contains("subtitleTrack");
				break;
			case WasPlayedRole:
				l_result = wasPlayed(l_file);
				break;
			case SelectedRole:
				l_result = l_file.id == m_selected;
				break;
		}
	}
	return l_result;
}

void dp::library::FileList::setFileList(const QVector<File>& files, unsigned long long playlistId) {
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
	beginResetModel();
	m_selected = m_paths[path];
	Q_EMIT fileSelected(path);
	endResetModel();
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
