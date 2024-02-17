#pragma once

#include <QString>
#include <QPointer>
#include <QThread>
#include <QUrl>
#include "../data/types.h"
#include "files_worker.h"

namespace dp {
namespace app {class App;}
namespace library {

class PlaylistController : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QVariantMap progress READ getProgress NOTIFY progressChanged)

public:
	static PlaylistController* instance();

	Q_SLOT void addFiles(const QList<QUrl>& paths);
	Q_SLOT void removeFile(unsigned long long id);
	Q_SLOT void addPlaylist();
	Q_SLOT void updateLabel(unsigned long long id, const QString& label);
	Q_SLOT void removePlaylist(unsigned long long id);

	QVariantMap getProgress();
	Q_SIGNAL void progressChanged();

private:
	static QPointer<PlaylistController> m_instance;
	QThread* m_workerThread;
	FilesWorker* m_worker;
	QVariantMap m_progress;

	explicit PlaylistController(QObject *parent = nullptr);
	~PlaylistController();
	static void init(QObject *parent = nullptr);

	Q_SIGNAL void addFilesToPlaylist(const QList<QUrl>& paths, unsigned long long playlistId);
	Q_SIGNAL void removeFileFromPlaylist(unsigned long long playlistId, unsigned long long fileId);
	Q_SIGNAL void createPlaylist(const QString& label);
	Q_SIGNAL void deletePlaylist(unsigned long long id);
	Q_SIGNAL void updatePlaylistName(unsigned long long id, const QString& label);
	Q_SLOT void handleError(const QString& error);
	Q_SLOT void handleProgress(QVariant current, QVariant work);

	Q_SIGNAL void error(const QString& error);

	friend class dp::app::App;
};
}}
