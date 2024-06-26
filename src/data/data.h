#pragma once

#include <QPointer>
#include <QSqlDatabase>
#include <QVariant>
#include <QVector>
#include "types.h"

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

	QVariant getFile(qulonglong id);

	QVariant getFile(const QString& path);
	QVariant createFile(const File& file);
	QVector<Playlist> getPlaylists();
	QVector<File> getFileList(qulonglong playlistId);

	QSqlDatabase m_db;
	Q_SIGNAL void error(const QString& error);

	friend class dp::app::App;
};
}}
