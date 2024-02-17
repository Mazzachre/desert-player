#pragma once

#include <QDir>
#include <QUrl>
#include <QList>
#include <QVariant>
#include <QMimeDatabase>
#include "../data/types.h"

namespace dp {
namespace library {

class FilesWorker : public QObject {
	Q_OBJECT

public:
	Q_SLOT void addFiles(const QList<QUrl>& paths, unsigned long long playlistId);
	Q_SIGNAL void progressUpdate(QVariant current, QVariant total);

	Q_SIGNAL void error(const QString& error);

	explicit FilesWorker(QObject* parent = nullptr);

private:
	QMimeDatabase m_db;

	QVariantMap m_fileTypes;
	QList<File> m_files;
	unsigned long long m_playlistId = 0;
	uint m_total = 0;
	uint m_done = 0;

	void fileMimeType(const QFileInfo& file);
	void walkDirectory(const QDir& dir);
	QVariantMap getMediaMeta(const QString& path);

	Q_SIGNAL void loadFiles(const QList<QString>& paths);
	Q_SLOT void filesLoaded(const QList<File>& files);

	Q_SIGNAL void createFiles(const QList<File>& files);
	Q_SLOT void filesCreated(const QList<File>& files);
	
	Q_SIGNAL void addToPlaylist(unsigned long long playlistId, const QVector<unsigned long long>& files);
};
}}
