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

	Q_SLOT void updatePlaylistName(uint id, const QString& label);
	Q_SLOT void removePlaylist(uint id);
	Q_SLOT void addPlaylist(const QString& label);
	Q_SIGNAL void playlistsUpdated(const QVector<Playlist>& playlists);

	Q_SLOT void removeFileFromPlaylist(const QString& path, uint playlistId);
	Q_SLOT void addFilesToPlaylist(const QList<File>& files, uint playlistId);
	Q_SLOT void playlistSelected(uint id);
	Q_SIGNAL void fileListUpdated(const QVector<File>& files, uint playlistId);

	Q_SLOT void loadFiles(const QList<QString>& paths);
	Q_SIGNAL void filesLoaded(const QList<File>& files);
	Q_SLOT void createFiles(const QList<File>& files);
	Q_SIGNAL void filesCreated(const QList<File>& files);

	Q_SLOT void updateTracks(const QString& path, const QVariantMap& tracks, uint playlistId);
	Q_SLOT void updatePlaybackData(const QString& path, const QVariantList& playbackData, uint playlistId);

private:
	static QPointer<Data> m_instance;
	Data(QObject* parent = nullptr);
	~Data();
	static void init(QObject* parent);
	void startup();

	QVariant getFile(const QString& path);
	QVariant createFile(const File& file);
	QVector<Playlist> getPlaylists();
	QVector<File> getFileList(uint playlistId);

	QSqlDatabase m_db;
	Q_SIGNAL void error(const QString& error);

	friend class dp::app::App;
};
}}
