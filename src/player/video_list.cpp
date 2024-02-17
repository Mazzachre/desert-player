#include "video_list.h"
#include <QDateTime>
#include <QTimeZone>
#include "../app/config.h"

QString videoLabel(const QVariant& title) {
	if (title.isValid()) return title.toString();
	return "unknown";
}

QPointer<dp::player::VideoList> dp::player::VideoList::m_instance = nullptr;

dp::player::VideoList::VideoList(QObject *parent)
	: QAbstractListModel(parent)
	, m_timer{new QTimer(this)} {
	connect(m_timer, &QTimer::timeout, this, &VideoList::hideContinue);	
}

void dp::player::VideoList::init(QObject *parent) {
	if (!m_instance) m_instance = new VideoList(parent);
}

dp::player::VideoList* dp::player::VideoList::instance() {
	if (!m_instance) qFatal("Video list accessed before initialized");
	return m_instance;
}

void dp::player::VideoList::setTracks(const QList<QVariant>& tracks) {
	beginResetModel();
	m_backing.clear();
	for (const auto& trackV: tracks) {
		QMap<QString, QVariant> track = trackV.toMap();
		QHash<int, QVariant> item;
		item[IdRole] = track["id"];
		item[CodecRole] = track["codec"];
		item[LabelRole] = videoLabel(track["title"]);
		m_backing.append(item);
	}
	endResetModel();	
}

void dp::player::VideoList::setSelected(const QVariant& trackId) {
	beginResetModel();
	m_selected = trackId;
	endResetModel();
}

void dp::player::VideoList::setPaused(bool paused) {
	m_paused = paused;
	Q_EMIT pausedChanged();
}

void dp::player::VideoList::setDuration(qulonglong duration) {
	m_duration = duration;
	Q_EMIT durationChanged();
}

void dp::player::VideoList::setPosition(qulonglong position) {
	m_position = position;
	Q_EMIT positionChanged();	
	if (position == 1 && m_continueData["position"].isValid()) {
		m_continueData["show"] = true;
		m_timer->start(dp::app::Config::instance()->showContinueFor());
		Q_EMIT continueDataChanged();
	}
}

QHash<int, QByteArray> dp::player::VideoList::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[CodecRole] = "codec";
    roles[LabelRole] = "label";
    roles[SelectedRole] = "selected";
    return roles;
}

int dp::player::VideoList::rowCount(const QModelIndex&) const {
	return m_backing.size();
}

QVariant dp::player::VideoList::data(const QModelIndex &index, int role) const {
	if (!index.isValid()) {
		return QVariant();
	}
    if (role == SelectedRole) {
		return m_backing[index.row()][IdRole] == m_selected;
	}
	return m_backing[index.row()][role];
}

bool dp::player::VideoList::getPaused() const {
	return m_paused;
}

qulonglong dp::player::VideoList::getDuration() const {
	return m_duration;
}

qulonglong dp::player::VideoList::getPosition() const {
	return m_position;
}

void dp::player::VideoList::changePosition(qulonglong position) const {
	Q_EMIT positionChange(position);
}

QVariantMap dp::player::VideoList::continueData() {
	return m_continueData;
}

void dp::player::VideoList::continuePlayback() {
	if (!m_continueData.isEmpty()) {
		Q_EMIT positionChange(m_continueData["position"].toULongLong());
		m_continueData["show"] = false;
		Q_EMIT continueDataChanged();
	}
}

void dp::player::VideoList::startPlayback(const File& file) {
	m_continueData = QVariantMap();
	m_continueData["show"] = false;
	if (!file.playbackData.isEmpty()) {
		QVariantMap l_continueData = file.playbackData.last().toMap();
		uint played = file.mediaMeta["duration"].isValid() ? (100* l_continueData["position"].toUInt()) / file.mediaMeta["duration"].toUInt() : 50;//If we don't have a duration we show this anyway, that is why the 50
		qlonglong l_now = QDateTime::currentDateTime().toSecsSinceEpoch();
		qlonglong l_started = l_continueData["started"].toLongLong();
		if (played < dp::app::Config::instance()->ignoreContinueAfter() && l_now - l_started < dp::app::Config::instance()->showContinueBefore()) {
			m_continueData["started"] = QDateTime::fromSecsSinceEpoch(l_started, QTimeZone::systemTimeZone()).toString(dp::app::Config::instance()->dateFormat() + " " + dp::app::Config::instance()->timeFormat());
			m_continueData["position"] = l_continueData["position"];
		}
	}
	Q_EMIT continueDataChanged();
}

void dp::player::VideoList::hideContinue() {
	m_continueData["show"] = false;
	Q_EMIT continueDataChanged();
}
