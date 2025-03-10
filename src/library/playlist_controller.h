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

	//New structure: Get files by path - Then filter by ID already in list - then add...
	Q_SLOT void addFiles(const QList<QUrl>& paths);
	Q_SLOT void addPlaylist();
	Q_SLOT void removeFile(qulonglong id);
	Q_SLOT void appendFiles(const QVector<qulonglong>& files);
	Q_SLOT void updateLabel(qulonglong id, const QString& label);
	Q_SLOT void removePlaylist(qulonglong id);
	//Move file one up or one down... That way we can make it sortable...
	Q_SLOT void moveFile(qulonglong id, int index);
	Q_SLOT void addExtras(qulonglong id, const QList<QUrl>& paths);

	QVariantMap getProgress();
	Q_SIGNAL void progressChanged();

private:
	static QPointer<PlaylistController> m_instance;
	QThread* m_workerThread;
	FilesWorker* m_worker;
	QVariantMap m_progress;
	qulonglong m_fileId;

	explicit PlaylistController(QObject *parent = nullptr);
	~PlaylistController();
	static void init(QObject *parent = nullptr);

	Q_SIGNAL void updatePlaylistFiles(qulonglong playlistId, QVector<qulonglong> files);
	Q_SIGNAL void createPlaylist(const QString& label);
	Q_SIGNAL void deletePlaylist(qulonglong id);
	Q_SIGNAL void updatePlaylistName(qulonglong id, const QString& label);
	Q_SIGNAL void createFiles(const QList<QUrl>& paths);
	Q_SLOT void handleError(const QString& error);
	Q_SLOT void handleProgress(QVariant current, QVariant work);

	Q_SIGNAL void error(const QString& error);

	friend class dp::app::App;
};
}}
