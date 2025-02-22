#pragma once

#include <QPointer>
#include <QSqlDatabase>
#include <QVariant>
#include <QVector>
#include "types.h"

namespace dp {
namespace data {

class Base : public QObject {
	Q_OBJECT

	explicit Base(QObject* parent = nullptr);
	~Base();

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

	Q_SLOT void startup();
	Q_SIGNAL void error(const QString& error);

	QVariant getFile(qulonglong id);

	QVariant getFile(const QString& path);
	QVariant createFile(const File& file);
	QVector<Playlist> getPlaylists();
	QVector<File> getFileList(qulonglong playlistId);

	QSqlDatabase m_db;

	friend class Data;
};
}}
