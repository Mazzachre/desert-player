#include "file_list.h"
#include <QTime>
#include "../app/config.h"

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
	roles[PathRole] = "path";
	roles[TitleRole] = "title";
	roles[DurationRole] = "duration";
	roles[HasSubtitleRole] = "hasSubtitle";
	roles[SelectedRole] = "selected";
	return roles;
}

int dp::library::FileList::rowCount(const QModelIndex&) const {
	return m_backing.size();	
}

QVariant dp::library::FileList::data(const QModelIndex &index, int role) const {
	QVariant l_result;
	if (index.isValid()) {
		File l_file = m_backing[index.row()];
		switch (role) {
			case PathRole:
				l_result = l_file.path;
				break;
			case TitleRole:
				//If we have year, season and episode we should add those too...
				l_result = l_file.mediaMeta.value("title");
				break;
			case DurationRole:
				l_result = l_file.mediaMeta.value("duration").isValid() ? QTime(0,0).addSecs(l_file.mediaMeta.value("duration").toUInt()).toString("H:mm:ss") : "??:??";
				break;
			case HasSubtitleRole:
				l_result = l_file.hasSubtitleStream(dp::app::Config::instance()->language(dp::app::Config::LanguageCodes::ISO_639_2)) || l_file.tracks.contains("subtitleTrack");
				break;
			case SelectedRole:
				l_result = l_file.path == m_selected;
				break;
		}
	}
	return l_result;
}

void dp::library::FileList::setFileList(const QVector<File>& files, uint playlistId) {
	beginResetModel();
	m_backing = files;
	Q_EMIT playableUpdated();
	if (m_backing.size() > 0) {
		if (playlistId != m_playlistId) {
			m_playlistId = playlistId;
			m_selected = m_backing[0].path;
		} else {
			auto it = std::find_if(m_backing.begin(), m_backing.end(), [&](File& item) {
				return item.path == m_selected;
			});
			if (it == m_backing.end()) {
				m_selected = m_backing[0].path;
			}
		}
	} else {
		m_selected = QString();
	}
	endResetModel();
}

void dp::library::FileList::selectFile(const QString& path) {
	beginResetModel();
	m_selected = path;
	Q_EMIT fileSelected(path);
	endResetModel();
}

void dp::library::FileList::startPlaying() {
	if (m_backing.size() > 0) {
		auto it = std::find_if(m_backing.begin(), m_backing.end(), [&](File& item) {
			return item.path == m_selected;
		});
		File* l_file = (it != m_backing.end()) ? it : m_backing.data();
		Q_EMIT playFile(*l_file);
	}
}

void dp::library::FileList::playPrev() {
	if (hasPrev()) {
		auto it = std::find_if(m_backing.begin(), m_backing.end(), [&](File& item) {
			return item.path == m_selected;
		});
		if (it != m_backing.end() && it != m_backing.begin()) {
			auto prev = std::prev(it);
			Q_EMIT playFile(*prev);
			return;
		}
	}
	Q_EMIT playlistFinished();
}

void dp::library::FileList::playNext() {
	if (hasNext()) {
		auto it = std::find_if(m_backing.begin(), m_backing.end(), [&](File& item) {
			return item.path == m_selected;
		});
		if (it != m_backing.end()) {
			auto next = std::next(it);
			if (next != m_backing.end()) {
				Q_EMIT playFile(*next);
				return;
			}
		}
	}
	Q_EMIT playlistFinished();
}

bool dp::library::FileList::hasPrev() {
	return m_backing.size() != 0 && m_selected != m_backing.first().path;
}

bool dp::library::FileList::hasNext() {
	return m_backing.size() != 0 && m_selected != m_backing.last().path;	
}

bool dp::library::FileList::getPlayable() {
	return m_backing.size() > 0;
}
