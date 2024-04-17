#include "playlist_controller.h"
#include "playlists.h"
#include <QDateTime>
#include "../app/config.h"

QPointer<dp::library::PlaylistController> dp::library::PlaylistController::m_instance = nullptr;

dp::library::PlaylistController::PlaylistController(QObject *parent)
	: QObject(parent)
	, m_workerThread{new QThread}
	, m_worker{new FilesWorker} {
	m_worker->moveToThread(m_workerThread);
	connect(this, &PlaylistController::createFiles, m_worker, &FilesWorker::addFiles);
	connect(m_worker, &FilesWorker::error, this, &PlaylistController::handleError);
	connect(m_worker, &FilesWorker::progressUpdate, this, &PlaylistController::handleProgress);
	connect(m_worker, &FilesWorker::filesAdded, this, &PlaylistController::appendFiles);
	connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
	m_workerThread->start();
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
	m_fileId = 0;
	Q_EMIT createFiles(paths);
}

void dp::library::PlaylistController::addExtras(qulonglong id, const QList<QUrl>& paths) {
	m_fileId = id;
	Q_EMIT createFiles(paths);
}

void dp::library::PlaylistController::handleError(const QString& errorString) {
	Q_EMIT error(errorString);
}

void dp::library::PlaylistController::appendFiles(const QVector<qulonglong>& files) {
	if (m_fileId == 0) {
		QVector<qulonglong> l_list(Playlists::instance()->getFiles());
		l_list << files;
		Q_EMIT updatePlaylistFiles(Playlists::instance()->getSelected(), l_list);
	} else {
		qDebug() << "Add extras" << m_fileId << files;
//		QVector<qulonglong> l_list(Data::instance()->getFileExtras(m_fileId));
//		l_list << files;
//		Data::instance()->setFileExtras(m_fileId, l_list);
		m_fileId = 0;
	}
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

void dp::library::PlaylistController::removePlaylist(qulonglong id) {
	Q_EMIT deletePlaylist(id);
}

void dp::library::PlaylistController::updateLabel(qulonglong id, const QString& label) {
	Q_EMIT updatePlaylistName(id, label);
}

void dp::library::PlaylistController::removeFile(qulonglong id) {
	QVector<qulonglong> l_list(Playlists::instance()->getFiles());
	l_list.removeOne(id);
	Q_EMIT updatePlaylistFiles(Playlists::instance()->getSelected(), l_list);
}

void dp::library::PlaylistController::moveFile(qulonglong id, int index) {
	QVector<qulonglong> l_list(Playlists::instance()->getFiles());
	int to = index < 0 ? l_list.size()-1 : index;
	l_list.move(Playlists::instance()->getFiles().indexOf(id), to);
	Q_EMIT updatePlaylistFiles(Playlists::instance()->getSelected(), l_list);
}
