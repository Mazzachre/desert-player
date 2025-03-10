#include "playlists.h"
#include <algorithm>

#include <QDebug>

QPointer<dp::library::Playlists> dp::library::Playlists::m_instance = nullptr;

dp::library::Playlists::Playlists(QObject *parent)
	: QAbstractListModel(parent) {}

dp::library::Playlists* dp::library::Playlists::instance() {
	if (!m_instance) qFatal("Playlists accessed before initalization");
	return m_instance;
}

void dp::library::Playlists::init(QObject *parent) {
	if (!m_instance) m_instance = new Playlists(parent);
}

QHash<int, QByteArray> dp::library::Playlists::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[SelectedRole] = "selected";
    roles[FileCountRole] = "fileCount";
    roles[LabelRole] = "label";
    return roles;
}

int dp::library::Playlists::rowCount(const QModelIndex&) const {
	return m_backing.size();
}

QVariant dp::library::Playlists::data(const QModelIndex &index, int role) const {
	QVariant l_result;
	if (index.isValid()) {
		switch (role) {
			case IdRole:
				l_result = m_backing[index.row()].id;
				break;
			case SelectedRole:
				l_result = m_backing[index.row()].id == m_selected;
				break;
			case FileCountRole:
				l_result = m_backing[index.row()].files.length();
				break;
			case LabelRole:
				l_result = m_backing[index.row()].label;
				break;
		}
	}
	return l_result;
}

int findIndex(const QVector<Playlist>& list, qulonglong id) {
	for(int i = 0; i < list.length(); ++i) {
		auto item = list.at(i);
		if (item.id == id) return i;
	}
	return -1;
}

void dp::library::Playlists::selectPlaylist(qulonglong id) {
	auto l_prev = index(findIndex(m_backing, m_selected), 0);
	auto l_next = index(findIndex(m_backing, id), 0);
    if (m_selected != id) {
		m_selected = id;
		Q_EMIT playlistSelected(id);
		Q_EMIT dataChanged(l_prev, l_prev, {SelectedRole});
		Q_EMIT dataChanged(l_next, l_next, {SelectedRole});
	}
}

void dp::library::Playlists::setPlaylists(const QVector<Playlist>& playlists) {
	beginResetModel();
	m_backing = playlists;
	auto it = std::find_if(m_backing.begin(), m_backing.end(), [&](Playlist& item) {
		return item.id == m_selected;
	});
	if (it == m_backing.end() || m_selected == 0) {
		m_selected = m_backing[0].id;
		Q_EMIT playlistSelected(m_selected);
	}
	endResetModel();
}

unsigned long long dp::library::Playlists::getSelected() {
	return m_selected;
}

QString dp::library::Playlists::getLabel() {
	auto it = std::find_if(m_backing.begin(), m_backing.end(), [&](Playlist& item) {
		return item.id == m_selected;
	});
	return it == m_backing.end() ? "" : it->label;
}

QVector<qulonglong> dp::library::Playlists::getFiles() {
	auto it = std::find_if(m_backing.begin(), m_backing.end(), [&](Playlist& item) {
		return item.id == m_selected;
	});
	return it == m_backing.end() ? QVector<qulonglong>() : it->files;	
}
