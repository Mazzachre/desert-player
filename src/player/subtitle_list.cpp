#include "subtitle_list.h"
#include <QDebug>

QString subLabel(const QVariant& language, const QVariant& title) {
	if (language.isValid() && title.isValid()) return language.toString() + " '" + title.toString() + "'";	
	if (language.isValid()) return language.toString();
	if (title.isValid()) return title.toString();
	return "unknown";
}

QPointer<dp::player::SubtitleList> dp::player::SubtitleList::m_instance = nullptr;

dp::player::SubtitleList::SubtitleList(QObject* parent)
	: QAbstractListModel(parent) {}

void dp::player::SubtitleList::init(QObject* parent) {
	if (!m_instance) m_instance = new SubtitleList(parent);
}

dp::player::SubtitleList* dp::player::SubtitleList::instance() {
	if (!m_instance) qFatal("Subtitle list accessed before initialized");
	return m_instance;
}

QHash<int, QByteArray> dp::player::SubtitleList::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[SelectedRole] = "selected";
    roles[LabelRole] = "label";
    roles[ExternalRole] = "external";
    roles[FilenameRole] = "filename";
    return roles;
}

int dp::player::SubtitleList::rowCount(const QModelIndex&) const {
	return m_backing.size();
}

QVariant dp::player::SubtitleList::data(const QModelIndex& index, int role) const {
	if (!index.isValid()) {
		return QVariant();
	}
    if (role == SelectedRole) {
		return m_backing[index.row()][IdRole] == m_selected;
	}
	return m_backing[index.row()][role];
}

void dp::player::SubtitleList::setTracks(const QList<QVariant>& tracks) {
	beginResetModel();
	m_backing.clear();
	for (const auto& trackV: tracks) {
		QMap<QString, QVariant> track = trackV.toMap();
		QHash<int, QVariant> item;
		item[IdRole] = track["id"];
		item[LabelRole] = subLabel(track["lang"], track["title"]);
		item[ExternalRole] = track["external"];
		item[FilenameRole] = track["external-filename"];
		m_backing.append(item);
	}
	endResetModel();	
}

void dp::player::SubtitleList::setSelected(const QVariant& trackId) {
	m_external = false;
	for (const auto& track: m_backing) {
		if (track[IdRole] == trackId) {
			m_external = true;
			break;
		}
	}
	Q_EMIT externalChanged();
	beginResetModel();
	m_selected = trackId;
	endResetModel();
}

void dp::player::SubtitleList::movieStarted(const QString& file) {
	QUrl url = QUrl::fromLocalFile(file);
	m_directory = url.adjusted(QUrl::RemoveFilename);
	Q_EMIT directoryChanged();
	m_fileName = url.fileName();
	Q_EMIT fileNameChanged();
}

QUrl dp::player::SubtitleList::getDirectory() const {
	return m_directory;
}

QString dp::player::SubtitleList::getFileName() const {
	return m_fileName;	
}

bool dp::player::SubtitleList::isExternal() const {
	return m_external;
}

void dp::player::SubtitleList::removeSubtitle() {
	Q_EMIT removeSubtitleTrack(m_selected);
}
