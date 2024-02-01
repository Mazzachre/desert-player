#include "playlist_controller.h"
#include "playlists.h"
#include <QDateTime>
#include "../app/config.h"
#include "../data/data.h"

QPointer<dp::library::PlaylistController> dp::library::PlaylistController::m_instance = nullptr;

dp::library::PlaylistController::PlaylistController(QObject *parent)
	: QObject(parent)
	, m_workerThread{new QThread}
	, m_worker{new FilesWorker} {
	m_worker->moveToThread(m_workerThread);
	connect(this, &PlaylistController::addFilesToPlaylist, m_worker, &FilesWorker::addFiles);
	connect(m_worker, &FilesWorker::error, this, &PlaylistController::handleError);
	connect(m_worker, &FilesWorker::progressUpdate, this, &PlaylistController::handleProgress);
	connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
	m_workerThread->start();
	connect(this, &PlaylistController::createPlaylist, dp::data::Data::instance(), &dp::data::Data::addPlaylist, Qt::QueuedConnection);
	connect(this, &PlaylistController::deletePlaylist, dp::data::Data::instance(), &dp::data::Data::removePlaylist, Qt::QueuedConnection);
	connect(this, &PlaylistController::removeFileFromPlaylist, dp::data::Data::instance(), &dp::data::Data::removeFileFromPlaylist, Qt::QueuedConnection);
}

dp::library::PlaylistController::~PlaylistController() {
	m_workerThread->quit();
	m_workerThread->wait();
	m_workerThread->deleteLater();	
}

void dp::library::PlaylistController::init(QObject *parent) {
	if (!m_instance) m_instance = new PlaylistController(parent);
}

dp::library::PlaylistController* dp::library::PlaylistController::instance() {
	if (!m_instance) qFatal("Files parser accessed before initialization");
	return m_instance;
}

void dp::library::PlaylistController::addFiles(const QList<QUrl>& paths) {
	Q_EMIT addFilesToPlaylist(paths, Playlists::instance()->getSelected());
}

void dp::library::PlaylistController::removeFile(const QString& path) {
	Q_EMIT removeFileFromPlaylist(path, Playlists::instance()->getSelected());
}

void dp::library::PlaylistController::handleError(const QString& errorString) {
	Q_EMIT error(errorString);
}

void dp::library::PlaylistController::handleProgress(QVariant current, QVariant work) {
	m_progress["value"] = current;
	m_progress["to"] = work;
	if (work.isValid()) {
		m_progress["show"] = true;
		if (work.toUInt() == 0) m_progress["indeterminate"] = true;
		else m_progress["indeterminate"] = false;
	} else m_progress["show"] = false;
	Q_EMIT progressChanged();
}

QVariantMap dp::library::PlaylistController::getProgress() {
	return m_progress;
}

void dp::library::PlaylistController::addPlaylist() {
	QString l_label = QDateTime::currentDateTime().toString(dp::app::Config::instance()->dateFormat() + " - " + dp::app::Config::instance()->timeFormat());
	Q_EMIT createPlaylist(l_label);
}

void dp::library::PlaylistController::removePlaylist(uint id) {
	Q_EMIT deletePlaylist(id);
}

void dp::library::PlaylistController::updateLabel(uint id, const QString& label) {
	Q_EMIT updatePlaylistName(id, label);
}
