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
	const QString& _path,
	const QVariantMap& _mediaMeta,
	const QVariantMap& _tracks,
	const QVariantList& _playbackData
) {
	path = _path;
	mediaMeta = _mediaMeta;
	tracks = _tracks;
	playbackData = _playbackData;
}

File::File(const File& other) {
	path = other.path;
	mediaMeta = other.mediaMeta;
	tracks = other.tracks;
	playbackData = other.playbackData;
}

File& File::operator=(const File& other) {
	path = other.path;
	mediaMeta = other.mediaMeta;
	tracks = other.tracks;
	playbackData = other.playbackData;
	return *this;
}

bool File::isValid() {
	return !path.isEmpty();
}

QDebug operator<<(QDebug dbg, const File& file) {
	//TODO Perhaps this should be better?
	dbg << "File:" << file.path << ":" << file.mediaMeta << file.tracks << file.playbackData;
	return dbg;
}

Playlist::Playlist() {}

Playlist::Playlist(uint _id, const QString& _label) {
	id = _id;
	label = _label;
}

QDebug operator<<(QDebug dbg, const Playlist& playlist) {
	dbg << "Playlist:" << playlist.id << "\"" << playlist.label << "\"";
	return dbg;
}

bool File::hasSubtitleStream(const QString& language) {
	QList<QVariant> l_streams = mediaMeta.value("streams").toList();
	for (auto& l_stream_entry : l_streams) {
		QVariantMap l_stream = l_stream_entry.toMap();
		if (l_stream.value("type") == QStringLiteral("subtitle")) {
			if (l_stream.value("language") == language) return true;
			if (l_stream.value("language") == QStringLiteral("unknown")) return true;
		}
	}
	return false;
}

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
