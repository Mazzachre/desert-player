#include "files_worker.h"
#include "../data/data.h"
#include "../app/config.h"
#include "../files/meta.h"

dp::library::FilesWorker::FilesWorker(QObject* parent)
	:QObject(parent) {
	connect(this, &FilesWorker::loadFiles, dp::data::Data::instance(), &dp::data::Data::loadFiles, Qt::QueuedConnection);
	connect(dp::data::Data::instance(), &dp::data::Data::filesLoaded, this, &FilesWorker::filesLoaded, Qt::QueuedConnection);
	connect(this, &FilesWorker::createFiles, dp::data::Data::instance(), &dp::data::Data::createFiles, Qt::QueuedConnection);
	connect(dp::data::Data::instance(), &dp::data::Data::filesCreated, this, &FilesWorker::filesCreated, Qt::QueuedConnection);
}

void dp::library::FilesWorker::fileMimeType(const QFileInfo& file) {
	QVariant l_file;
	QMimeType type = m_db.mimeTypeForFile(file);
	m_fileTypes[file.absoluteFilePath()] = type.name();
}

void dp::library::FilesWorker::walkDirectory(const QDir& dir) {
	for (auto& info : dir.entryInfoList(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot)) {
		if (info.isDir()) {
			walkDirectory(QDir(info.absoluteFilePath()));
		} else if (info.isFile() && info.isReadable()) {
			fileMimeType(info);
		} else {
			Q_EMIT error(QStringLiteral("Error opening: ") + info.absoluteFilePath());
		}
	}
}

void dp::library::FilesWorker::addFiles(const QList<QUrl>& paths) {
	if (m_total != 0) {
		Q_EMIT error("Files worker already adding files");
		return;
	}
	m_done = 0;
	m_total = 1;
	Q_EMIT progressUpdate(m_done, m_total);
	
	for (auto& path : paths ) {
		if (path.isLocalFile()) {
			QFileInfo info(path.toLocalFile());
			if (info.isDir()) {
				walkDirectory(QDir(info.absoluteFilePath()));
			} else if (info.isFile() && info.isReadable()) {
				fileMimeType(info);
			} else {
				Q_EMIT error(QStringLiteral("Error opening: ") + info.absoluteFilePath());
			}
		} else {
			Q_EMIT error(QStringLiteral("Cannot add: ") + path.toString());
		}
	}
	if (!m_fileTypes.isEmpty()) {
		QList<QString> l_paths;
		for (auto& path : m_fileTypes.keys()) {
			if (m_fileTypes[path].toString().startsWith(QStringLiteral("video"))) {
				l_paths << path;
				m_total++;
				Q_EMIT progressUpdate(m_done, m_total);
			}
		}
		Q_EMIT loadFiles(l_paths);
	}
}

void dp::library::FilesWorker::filesLoaded(const QList<File>& files) {
	QList<QString> l_paths;
	for (auto& path : m_fileTypes.keys()) {
		if (m_fileTypes[path].toString().startsWith(QStringLiteral("video"))) {
			l_paths << path;
		}
	}
	for (auto& l_file : files) {
		m_files << l_file;
		l_paths.removeOne(l_file.path);
		m_done++;
		Q_EMIT progressUpdate(m_done, m_total);
	}
	QList<File> l_files;
	for (auto& path : l_paths) {
		QVariantMap l_metaData = getMediaMeta(path);
		QVariantMap l_tracks;
		l_metaData["mime"] = m_fileTypes[path];
		uint len = path.lastIndexOf("/");
		for (auto& subtitle : m_fileTypes.keys()) {
			if (m_fileTypes[subtitle] == QStringLiteral("application/x-subrip") && subtitle.startsWith(path.left(len))) {
				QString subPath = subtitle.mid(len+1);
				uint sub_len = subPath.indexOf("/");
				QString subDirPath = subPath.left(sub_len).toLower();
				QString fileName = path.mid(len+1).chopped(4);
				if (subDirPath == QStringLiteral("subs") || subDirPath == QStringLiteral("subtitles")) {
					if (subPath.mid(sub_len+1).chopped(4).contains(dp::app::Config::instance()->language(dp::app::Config::LanguageCodes::ISO_639_2), Qt::CaseInsensitive)) {
						l_tracks["subtitleTrack"] = subtitle;
					}
				} else if (subPath.startsWith(fileName)) {
					if (subtitle.mid(len+fileName.size()) == QStringLiteral(".srt")) {
						l_tracks["subtitleTrack"] = subtitle;
					} else if (subtitle.mid(len+fileName.size()).contains(dp::app::Config::instance()->language(dp::app::Config::LanguageCodes::ISO_639_2), Qt::CaseInsensitive)) {
						l_tracks["subtitleTrack"] = subtitle;
					}
				}
			}
		}
		l_files << File(0, path, l_metaData, l_tracks, QVariantList());
		m_done++;
		Q_EMIT progressUpdate(m_done, m_total);
	}
	if (!l_files.isEmpty()) {
		Q_EMIT createFiles(l_files);
	} else {
		filesCreated(QList<File>());
	}
}

void dp::library::FilesWorker::filesCreated(const QList<File>& files) {
	QVector<qulonglong> l_files;
	for (auto& l_file : m_files) {
		l_files << l_file.id;
	}
	for (auto& l_file : files) {
		l_files << l_file.id;
	}
	Q_EMIT filesAdded(l_files);
	
	m_fileTypes.clear();
	m_files.clear();
	m_total = 0;
	m_done = 0;
	Q_EMIT progressUpdate(QVariant(), QVariant());
}
