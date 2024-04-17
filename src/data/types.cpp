#include "types.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QFileInfo>

QVariantMap findSeasonAndEpisode(const QString& fileName) {
	QRegularExpression l_matchEpisode("S(\\d{1,3})\\W+E(\\d{1,3})|(\\d{1,3})x(\\d{1,3})");
	QRegularExpressionMatch l_episodeMatch = l_matchEpisode.match(fileName);
	if (!l_episodeMatch.hasMatch()) return QVariantMap();
	int l_season = std::max(l_episodeMatch.captured(1).toUInt(), l_episodeMatch.captured(3).toUInt());
	int l_episode = std::max(l_episodeMatch.captured(2).toUInt(), l_episodeMatch.captured(4).toUInt());
	return QVariantMap({{"start", l_episodeMatch.capturedStart()}, {"season", l_season}, {"episode", l_episode}});
}

//TODO This should find the last one...
QVariantMap findYear(const QString& fileName) {
	QRegularExpression l_matchYear("(19|20)\\d{2}");
	QRegularExpressionMatch l_yearMatch = l_matchYear.match(fileName, 1);
	if (!l_yearMatch.hasMatch()) return QVariantMap();
	return QVariantMap({{"start", l_yearMatch.capturedStart()}, {"year", l_yearMatch.captured()}});
}

QVariantMap findQuery(const QString& fileName) {
	QVariantMap l_episode = findSeasonAndEpisode(fileName);
	QVariantMap l_year = findYear(fileName);

	QString l_query = fileName.left(std::min(!l_episode.isEmpty() ? l_episode["start"].toInt() : fileName.length(), !l_year.isEmpty() ? l_year["start"].toInt() : fileName.length())).trimmed().split(QRegularExpression("\\W"), QString::SkipEmptyParts).join(" ");
	QString l_type = (!l_episode.isEmpty()) ? "TV Show" : "Movie";

	return QVariantMap({
		{"query", l_query},
		{"type", l_type},
		{"year", !l_year.isEmpty() ? l_year["year"] : ""},
	});
}

File::File() {}

File::File(
	unsigned long long _id,
	const QString& _path,
	const QVariantMap& _mediaMeta,
	const QVariantMap& _tracks,
	const QVariantList& _playbackData
) {
	id = _id;
	path = _path;
	mediaMeta = _mediaMeta;
	tracks = _tracks;
	playbackData = _playbackData;
}

File::File(const File& other) {
	id = other.id;
	path = other.path;
	mediaMeta = other.mediaMeta;
	tracks = other.tracks;
	playbackData = other.playbackData;
}

File& File::operator=(const File& other) {
	id = other.id;
	path = other.path;
	mediaMeta = other.mediaMeta;
	tracks = other.tracks;
	playbackData = other.playbackData;
	return *this;
}

bool File::isValid() const {
	return id > 0 && !path.isEmpty();
}

bool File::hasInternalSubtitleStream() const {
	QList<QVariant> l_streams = mediaMeta.value("streams").toList();
	for (auto& l_stream_entry : l_streams) {
		QVariantMap l_stream = l_stream_entry.toMap();
		if (l_stream.value("type") == QStringLiteral("subtitle")) return true;		
	}
	return false;
}

QString File::getSubtitleLabel(uint id) const {
	QList<QVariant> l_streams = mediaMeta.value("streams").toList();
	for (auto& l_stream_entry : l_streams) {
		QVariantMap l_stream = l_stream_entry.toMap();
		if (l_stream.value("type") == QStringLiteral("subtitle") && l_stream.value("id") == id) {
			if (l_stream.value("language").isValid() && l_stream.value("title").isValid()) return l_stream.value("language").toString() + " '" + l_stream.value("title").toString() + "'";
			if (l_stream.value("language").isValid()) return l_stream.value("language").toString();
			if (l_stream.value("title").isValid()) return l_stream.value("title").toString();
			return "Unknown";
		};
	}
	return "Unknown";
}

qulonglong File::getDurationPercentage(uint percentage) const {
	//default to an hour if we don't have duration data
	return (mediaMeta["duration"].isValid() ? mediaMeta["duration"].toUInt() : 3600)* percentage / 100;
}

QDebug operator<<(QDebug dbg, const File& file) {
	//TODO Perhaps this should be better?
	dbg << "File:" << file.id << file.path << ":" << file.mediaMeta << file.tracks << file.playbackData;
	return dbg;
}

Playlist::Playlist() {}

Playlist::Playlist(unsigned long long _id, const QString& _label, const QVector<unsigned long long>& _files) {
	id = _id;
	label = _label;
	files = _files;
}

QDebug operator<<(QDebug dbg, const Playlist& playlist) {
	dbg << "Playlist:" << playlist.id << "\"" << playlist.label << "\"";
	return dbg;
}

/*
QVariantMap File::getQueryData() {
	QFileInfo l_fi(path);
	QString l_title = l_fi.fileName().chopped(4);
	return findQuery(l_title);
}

QVariantMap File::getEpisodeData(const QString& path) {
	QFileInfo l_fi(path);
	QString l_title = l_fi.fileName().chopped(4);
	return findSeasonAndEpisode(l_title);
}
*/
