#pragma once

#include <QString>
#include <QVariant>
#include <QDebug>

/*
 * mediaMeta: title, duration, streams[type, <video: codec, width>, <audio: codec, channels, language>, <subtitle: language>]
 * tracks: videoTrack<num>, audioTrack<num>, subtitleTrack<num/string>, volume, skip-list
 * playbackData: [position, started, accomulated]
 */
struct File {
	unsigned long long id;
	QString path;
	QVariantMap mediaMeta;
	QVariantMap tracks;
	QVariantList playbackData;
	explicit File();
	explicit File(
		unsigned long long id,
		const QString& path,
		const QVariantMap& mediaMeta,
		const QVariantMap& tracks,
		const QVariantList& playbackData
	);
	File(const File& other);
	File& operator=(const File& other);
	bool isValid();
	bool hasSubtitleStream(const QString& language);
	QVariantMap getQueryData();
	static QVariantMap getEpisodeData(const QString& path);
};
Q_DECLARE_METATYPE(File);
QDebug operator<<(QDebug dbg, const File& file);

struct Playlist {
	unsigned long long id;
	QString label;
	QVector<unsigned long long> files;
	explicit Playlist();
	explicit Playlist(unsigned long long id, const QString& label, const QVector<unsigned long long>& files);
};
Q_DECLARE_METATYPE(Playlist);
QDebug operator<<(QDebug dbg, const Playlist& playlist);
