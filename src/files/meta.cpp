#include "meta.h"

#include <QFileInfo>

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
	QVariantMap meta = mapMeta(stream->metadata);
	
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

QVariantMap getMediaMeta(const QString& path) {
	QVariantMap l_result;
	AVFormatContext* m_formatCtx = nullptr;
	int err = avformat_open_input(&m_formatCtx, path.toUtf8().constData(), NULL, NULL);
	if (err == 0) {
		uint duration = 0;
		QVariantMap meta = mapMeta(m_formatCtx->metadata);
		QFileInfo l_fi(path);
		l_result["title"] = meta.value("title", l_fi.fileName().chopped(4));
		QList<QVariant> streams;
		QMap<QString, int> streamIds = {{"video", 1}, {"audio", 1}, {"subtitle", 1}};
		for (uint i = 0; i < m_formatCtx->nb_streams; ++i) {
			QVariantMap stream = streamData(m_formatCtx->streams[i], duration);
			if (!stream.isEmpty()) {
				int id = streamIds[stream["type"].toString()];
				stream["id"] = id;
				streamIds[stream["type"].toString()] = id+1;
				streams << stream;
			}
		}
		l_result["streams"] = QVariant(streams);
		if (m_formatCtx->duration > 0 ) {
			l_result["duration"] = QVariant::fromValue(m_formatCtx->duration / AV_TIME_BASE);
		} else if (duration > 0) {
			l_result["duration"] = duration;
		}
		avformat_free_context(m_formatCtx);
	} else {
//		char* errStr = new char[AV_ERROR_MAX_STRING_SIZE];
//		Q_EMIT error("Error reading meta data: " + QString(av_make_error_string(errStr, AV_ERROR_MAX_STRING_SIZE, err)));
//		delete[] errStr;

	}
	return l_result;
}
