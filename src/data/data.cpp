#include "data.h"

QPointer<dp::data::Data> dp::data::Data::m_instance = nullptr;

dp::data::Data* dp::data::Data::instance() {
	if (!m_instance) qFatal("Database accessed before initialized");
	return m_instance;
}

dp::data::Data::Data(QObject *parent)
	: QObject(parent)
	, m_base{new Base}
	, m_baseThread{new QThread} {
	m_baseThread->start();
	m_base->moveToThread(m_baseThread);

	connect(this, &dp::data::Data::doStartup, m_base, &dp::data::Base::startup, Qt::QueuedConnection);
	connect(this, &dp::data::Data::doAddPlaylist, m_base, &dp::data::Base::addPlaylist, Qt::QueuedConnection);
	connect(this, &dp::data::Data::doUpdatePlaylistName, m_base, &dp::data::Base::updatePlaylistName, Qt::QueuedConnection);
	connect(this, &dp::data::Data::doRemovePlaylist, m_base, &dp::data::Base::removePlaylist, Qt::QueuedConnection);
	connect(this, &dp::data::Data::doUpdatePlaylistFiles, m_base, &dp::data::Base::updatePlaylistFiles, Qt::QueuedConnection);
	connect(this, &dp::data::Data::doPlaylistSelected, m_base, &dp::data::Base::playlistSelected, Qt::QueuedConnection);

	connect(this, &dp::data::Data::doLoadFiles, m_base, &dp::data::Base::loadFiles, Qt::QueuedConnection);
	connect(this, &dp::data::Data::doCreateFiles, m_base, &dp::data::Base::createFiles, Qt::QueuedConnection);
	connect(this, &dp::data::Data::doUpdateTracks, m_base, &dp::data::Base::updateTracks, Qt::QueuedConnection);
	connect(this, &dp::data::Data::doUpdatePlaybackData, m_base, &dp::data::Base::updatePlaybackData, Qt::QueuedConnection);
	connect(this, &dp::data::Data::doUpdateMetaData, m_base, &dp::data::Base::updateMetaData, Qt::QueuedConnection);

	connect(m_base, &dp::data::Base::playlistsUpdated, this, &dp::data::Data::handlePlaylistsUpdated, Qt::QueuedConnection);
	connect(m_base, &dp::data::Base::fileListUpdated, this, &dp::data::Data::handleFileListUpdated, Qt::QueuedConnection);
	connect(m_base, &dp::data::Base::filesLoaded, this, &dp::data::Data::handleFilesLoaded, Qt::QueuedConnection);
	connect(m_base, &dp::data::Base::filesCreated, this, &dp::data::Data::handleFilesCreated, Qt::QueuedConnection);
	connect(m_base, &dp::data::Base::fileUpdated, this, &dp::data::Data::handleFileUpdated, Qt::QueuedConnection);
}

dp::data::Data::~Data() {
	m_baseThread->quit();
	m_baseThread->wait();
	m_baseThread->deleteLater();
	m_base->deleteLater();
}

void dp::data::Data::init(QObject *parent) {
	if (!m_instance) m_instance = new Data(parent);
}

void dp::data::Data::startup() {
	Q_EMIT doStartup();
}

void dp::data::Data::updatePlaylistName(qulonglong playlistId, const QString& label) {
	Q_EMIT doUpdatePlaylistName(playlistId, label);
}

void dp::data::Data::removePlaylist(qulonglong id) {
	Q_EMIT doRemovePlaylist(id);
}

void dp::data::Data::addPlaylist(const QString& label) {
	Q_EMIT doAddPlaylist(label);
}

void dp::data::Data::updatePlaylistFiles(qulonglong playlistId, QVector<qulonglong> files) {
	Q_EMIT doUpdatePlaylistFiles(playlistId, files);
}

void dp::data::Data::playlistSelected(qulonglong playlistId) {
	Q_EMIT doPlaylistSelected(playlistId);
}

void dp::data::Data::loadFiles(const QList<QString>& paths) {
	Q_EMIT doLoadFiles(paths);
}

void dp::data::Data::createFiles(const QList<File>& files) {
	Q_EMIT doCreateFiles(files);
}

void dp::data::Data::updateTracks(qulonglong fileId, const QVariantMap& tracks) {
	Q_EMIT doUpdateTracks(fileId, tracks);
}

void dp::data::Data::updatePlaybackData(qulonglong fileId, const QVariantList& playbackData) {
	Q_EMIT doUpdatePlaybackData(fileId, playbackData);
}

void dp::data::Data::updateMetaData(qulonglong fileId, const QVariantMap& mediaMeta) {
	Q_EMIT doUpdateMetaData(fileId, mediaMeta);
}

void dp::data::Data::handlePlaylistsUpdated(const QVector<Playlist>& playlists) {
	Q_EMIT playlistsUpdated(playlists);
}

void dp::data::Data::handleFileListUpdated(const QVector<File>& files, qulonglong playlistId) {
	Q_EMIT fileListUpdated(files, playlistId);
}

void dp::data::Data::handleFilesLoaded(const QList<File>& files) {
	Q_EMIT filesLoaded(files);
}

void dp::data::Data::handleFilesCreated(const QList<File>& files) {
	Q_EMIT filesCreated(files);
}

void dp::data::Data::handleFileUpdated(const File& file) {
	Q_EMIT fileUpdated(file);
}

void dp::data::Data::handleError(const QString& err) {
	Q_EMIT error(err);
}
