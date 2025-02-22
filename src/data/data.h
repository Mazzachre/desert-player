#pragma once

#include <QPointer>
#include <QVariant>
#include <QVector>
#include <QThread>
#include "types.h"
#include "base.h"

namespace dp {
namespace app {class App;}
namespace data {

class Data : public QObject {
	Q_OBJECT

public:
	static Data* instance();

	Q_SLOT void addPlaylist(const QString& label);
	Q_SLOT void updatePlaylistName(qulonglong id, const QString& label);
	Q_SLOT void removePlaylist(qulonglong id);
	Q_SLOT void updatePlaylistFiles(qulonglong playlistId, QVector<qulonglong> files);
	Q_SLOT void playlistSelected(qulonglong id);
	Q_SIGNAL void playlistsUpdated(const QVector<Playlist>& playlists);
	Q_SIGNAL void fileListUpdated(const QVector<File>& files, qulonglong playlistId);

	Q_SLOT void loadFiles(const QList<QString>& paths);
	Q_SIGNAL void filesLoaded(const QList<File>& files);
	Q_SLOT void createFiles(const QList<File>& files);
	Q_SIGNAL void filesCreated(const QList<File>& files);

	Q_SLOT void updateTracks(qulonglong fileId, const QVariantMap& tracks);
	Q_SLOT void updatePlaybackData(qulonglong fileId, const QVariantList& playbackData);
	Q_SLOT void updateMetaData(qulonglong fileId, const QVariantMap& metaData);
	Q_SIGNAL void fileUpdated(const File& file);

	//Update file with extras list...

private:
	static QPointer<Data> m_instance;
	Data(QObject* parent = nullptr);
	~Data();
	static void init(QObject* parent);
	void startup();
	Q_SIGNAL void doStartup();

	Base* m_base{nullptr};
	QThread* m_baseThread{nullptr};

	Q_SIGNAL void doAddPlaylist(const QString& label);
	Q_SIGNAL void doUpdatePlaylistName(qulonglong id, const QString& label);
	Q_SIGNAL void doRemovePlaylist(qulonglong id);
	Q_SIGNAL void doUpdatePlaylistFiles(qulonglong playlistId, QVector<qulonglong> files);
	Q_SIGNAL void doPlaylistSelected(qulonglong id);

	Q_SIGNAL void doLoadFiles(const QList<QString>& paths);
	Q_SIGNAL void doCreateFiles(const QList<File>& files);
	Q_SIGNAL void doUpdateTracks(qulonglong fileId, const QVariantMap& tracks);
	Q_SIGNAL void doUpdatePlaybackData(qulonglong fileId, const QVariantList& playbackData);
	Q_SIGNAL void doUpdateMetaData(qulonglong fileId, const QVariantMap& metaData);

	Q_SLOT void handlePlaylistsUpdated(const QVector<Playlist>& playlists);
	Q_SLOT void handleFileListUpdated(const QVector<File>& files, qulonglong playlistId);	
	Q_SLOT void handleFilesLoaded(const QList<File>& files);
	Q_SLOT void handleFilesCreated(const QList<File>& files);
	Q_SLOT void handleFileUpdated(const File& file);

	Q_SLOT void handleError(const QString& error);
	Q_SIGNAL void error(const QString& error);

	friend class dp::app::App;
};
}}
