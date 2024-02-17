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
	Q_SLOT void updatePlaylistName(unsigned long long id, const QString& label);
	Q_SLOT void removePlaylist(unsigned long long id);
	Q_SLOT void updatePlaylistFiles(unsigned long long playlistId, QVector<unsigned long long> files);
	Q_SLOT void appendPlaylistFiles(unsigned long long playlistId, QVector<unsigned long long> files);
	Q_SLOT void deletePlaylistFile(unsigned long long playlistId, unsigned long long file);
	Q_SLOT void playlistSelected(unsigned long long id);
	Q_SIGNAL void playlistsUpdated(const QVector<Playlist>& playlists);
	Q_SIGNAL void fileListUpdated(const QVector<File>& files, unsigned long long playlistId);

	Q_SLOT void loadFiles(const QList<QString>& paths);
	Q_SIGNAL void filesLoaded(const QList<File>& files);
	Q_SLOT void createFiles(const QList<File>& files);
	Q_SIGNAL void filesCreated(const QList<File>& files);

	Q_SLOT void updateTracks(const QString& path, const QVariantMap& tracks, unsigned long long playlistId);
	Q_SLOT void updatePlaybackData(const QString& path, const QVariantList& playbackData, unsigned long long playlistId);

private:
	static QPointer<Data> m_instance;
	Data(QObject* parent = nullptr);
	~Data();
	static void init(QObject* parent);
	void startup();

	QVariant getFile(const QString& path);
	QVariant createFile(const File& file);
	QVector<Playlist> getPlaylists();
	QVector<File> getFileList(unsigned long long playlistId);

	QSqlDatabase m_db;
	Q_SIGNAL void error(const QString& error);

	friend class dp::app::App;
};
}}
