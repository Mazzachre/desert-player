#include "audio_list.h"

QString channelLabel(const QVariant& channelCount) {
	if (channelCount.isNull()) return "";
	int channels = channelCount.toInt();
	if (channels == 0) return "";
	if (channels == 1) return "mono";
	if (channels == 2) return "stereo";
	return "surround";
}

QString audioLabel(const QVariant& language, const QVariant& title) {
	if (language.isValid() && title.isValid()) return language.toString() + " '" + title.toString() + "'";	
	if (language.isValid()) return language.toString();
	if (title.isValid()) return title.toString();
	return "unknown";
}

QPointer<dp::player::AudioList> dp::player::AudioList::m_instance = nullptr;

dp::player::AudioList::AudioList(QObject* parent)
	: QAbstractListModel(parent) {}

void dp::player::AudioList::init(QObject* parent) {
	if (!m_instance) m_instance = new AudioList(parent);
}

dp::player::AudioList* dp::player::AudioList::instance() {
	if (!m_instance) qFatal("Audio list accessed before initialized");
	return m_instance;
}

QHash<int, QByteArray> dp::player::AudioList::roleNames() const {
	QHash<int, QByteArray> roles;
	roles[IdRole] = "id";
	roles[SelectedRole] = "selected";
	roles[LabelRole] = "label";
	roles[CodecRole] = "codec";
	roles[ChannelsRole] = "channels";
	return roles;
}

int dp::player::AudioList::rowCount(const QModelIndex&) const {
	return m_backing.size();
}

QVariant dp::player::AudioList::data(const QModelIndex& index, int role) const {
	if (!index.isValid()) {
		return QVariant();
	}
    if (role == SelectedRole) {
		return m_backing[index.row()][IdRole] == m_selected;
	}
	return m_backing[index.row()][role];	
}

void dp::player::AudioList::setMuted(bool muted) {
	m_muted = muted;
	Q_EMIT mutedChanged();
}

bool dp::player::AudioList::getMuted() const {
	return m_muted;
}

void dp::player::AudioList::setVolume(uint volume) {
	m_volume = volume;
	Q_EMIT volumeChanged();
}

uint dp::player::AudioList::getVolume() const {
	return m_volume;
}

void dp::player::AudioList::changeVolume(uint volume) const {
	Q_EMIT volumeChange(volume);
}

void dp::player::AudioList::setTracks(const QList<QVariant>& tracks) {
	beginResetModel();
	m_backing.clear();
	for (const auto& trackV: tracks) {
		QMap<QString, QVariant> track = trackV.toMap();
		QHash<int, QVariant> item;
		item[IdRole] = track["id"];
		item[LabelRole] = audioLabel(track["lang"], track["title"]);
		item[CodecRole] = track["codec"];
		item[ChannelsRole] = channelLabel(track["demux-channel-count"]);
		m_backing.append(item);
	}
	endResetModel();
}

void dp::player::AudioList::setSelected(const QVariant& trackId) {
	beginResetModel();
	m_selected = trackId;
	endResetModel();
}
