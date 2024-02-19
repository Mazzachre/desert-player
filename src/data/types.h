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
	qulonglong id;
	QString path;
	QVariantMap mediaMeta;
	QVariantMap tracks;
	QVariantList playbackData;
	explicit File();
	explicit File(
		qulonglong id,
		const QString& path,
		const QVariantMap& mediaMeta,
		const QVariantMap& tracks,
		const QVariantList& playbackData
	);
	File(const File& other);
	File& operator=(const File& other);
	bool isValid() const;
	bool hasSubtitleStream(const QString& language) const;
	qulonglong getDurationPercentage(uint percentage) const;
};
Q_DECLARE_METATYPE(File);
QDebug operator<<(QDebug dbg, const File& file);

struct Playlist {
	qulonglong id;
	QString label;
	QVector<qulonglong> files;
	explicit Playlist();
	explicit Playlist(qulonglong id, const QString& label, const QVector<qulonglong>& files);
};
Q_DECLARE_METATYPE(Playlist);
QDebug operator<<(QDebug dbg, const Playlist& playlist);
