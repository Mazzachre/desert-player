#include "files_worker.h"
#include "../data/data.h"
#include "../app/config.h"
extern "C" {
#include <libavformat/avformat.h>
}

QVariantMap mapMeta(AVDictionary* dict) {
	QVariantMap l_data;
	AVDictionaryEntry *tag = nullptr;
	while ((tag = av_dict_get(dict, "", tag, AV_DICT_IGNORE_SUFFIX))) {
		l_data[tag->key] = tag->value;
	}
	return l_data;
}

QVariantMap streamData(AVStream* stream, uint& duration) {
	QVariantMap l_data;
	//TODO I don't know what to put here!!! It should match the player so I can select?
	l_data["id"] = stream->index;
	QVariantMap meta = mapMeta(stream->metadata);
	
	qDebug() << "Media meta data:" << meta;
	
	if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
		duration = (stream->duration * stream->time_base.num) / stream->time_base.den;
		l_data["type"] = "video";
		l_data["codec"] = QString(avcodec_get_name(stream->codecpar->codec_id));
		l_data["width"] = stream->codecpar->width;
		if (meta.contains("title")) {
			l_data["title"] = meta.value("title");
		}
	} else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
		l_data["type"] = "audio";
		l_data["codec"] = QString(avcodec_get_name(stream->codecpar->codec_id));
		l_data["channels"] = stream->codecpar->channels;
		l_data["language"] = meta.value("language", "unknown");
		if (meta.contains("title")) {
			l_data["title"] = meta.value("title");
		}
	} else if (stream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE ) {
		l_data["type"] = "subtitle";
		l_data["language"] = meta.value("language", "unknown");
		if (meta.contains("title")) {
			l_data["title"] = meta.value("title");
		}
	}
	return l_data;
}

QVariantMap dp::library::FilesWorker::getMediaMeta(const QString& path) {
	QVariantMap l_result;
	AVFormatContext* m_formatCtx = nullptr;
	int err = avformat_open_input(&m_formatCtx, path.toUtf8().constData(), NULL, NULL);
	if (err == 0) {
		uint duration = 0;
		QVariantMap meta = mapMeta(m_formatCtx->metadata);
		QFileInfo l_fi(path);
		l_result["title"] = meta.value("title", l_fi.fileName().chopped(4));
		QList<QVariant> streams;
		for (uint i = 0; i < m_formatCtx->nb_streams; ++i) {
			QVariantMap stream = streamData(m_formatCtx->streams[i], duration);
			if (!stream.isEmpty()) streams << stream;
		}
		l_result["streams"] = QVariant(streams);
		if (m_formatCtx->duration > 0 ) {
			l_result["duration"] = QVariant::fromValue(m_formatCtx->duration / AV_TIME_BASE);
		} else if (duration > 0) {
			l_result["duration"] = duration;
		}
		avformat_free_context(m_formatCtx);
	} else {
		char* errStr = new char[AV_ERROR_MAX_STRING_SIZE];
		Q_EMIT error("Error reading meta data: " + QString(av_make_error_string(errStr, AV_ERROR_MAX_STRING_SIZE, err)));
		delete[] errStr;
	}
	return l_result;
}

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
	Q_EMIT addToPlaylist(l_files);
	
	m_fileTypes.clear();
	m_files.clear();
	m_total = 0;
	m_done = 0;
	Q_EMIT progressUpdate(QVariant(), QVariant());
}
