#include "base.h"
#include "../app/config.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

uint getVersion(const QSqlDatabase& db) {
	QSqlQuery query(db);
	if(!query.exec("CREATE TABLE IF NOT EXISTS version (version_number INT)")) qFatal("Get version Error: %s", db.lastError().text().toUtf8().constData());
	if (!query.exec("SELECT version_number FROM version")) qFatal("Get version Error: %s", db.lastError().text().toUtf8().constData());
	if (query.next()) {
		return query.value(0).toUInt();
	} else {
		if(!query.exec("INSERT INTO version (version_number) VALUES(0)")) qFatal("Get version Error: %s", db.lastError().text().toUtf8().constData());
	}
	return 0;
}

QString loadSqlScript(uint version) {
    QFile file(QString(":/sqlscripts/v_%1.sql").arg(version));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        return in.readAll();
    } else {
		qFatal("Unable to open database script :/sqlscripts/v_%d.sql", version);
	}
    return QString();
}

uint initDb(const QSqlDatabase& db, uint version) {
	QString script = loadSqlScript(version);
	if (script.isEmpty()) qFatal("Unable to load database script %d", version);

	QSqlQuery query(db);
	for (auto& line : script.split("\n")) {
		if (line.trimmed().size() > 0) {
			if (!query.exec(line)) qFatal("Sql Script Error: %s", query.lastError().text().toUtf8().constData());
		}
	}

	if (!query.exec(QString("UPDATE version SET version_number = %1").arg(version))) qFatal("Update version Error: %s", db.lastError().text().toUtf8().constData());
	return version;
}

void initDb(const QSqlDatabase& db) {
	uint currentVersion = getVersion(db);
	while(currentVersion < dp::app::Config::instance()->version()) {
		currentVersion = initDb(db, currentVersion + 1);
	}
}

File fileFromQuery(QSqlQuery entry) {
	return File(
		entry.value("rowid").toULongLong(),
		entry.value("path").toString(),
		QJsonDocument::fromJson(entry.value("media").toByteArray()).object().toVariantMap(),
		QJsonDocument::fromJson(entry.value("tracks").toByteArray()).object().toVariantMap(),
		QJsonDocument::fromJson(entry.value("playback").toByteArray()).array().toVariantList()
	);
}

dp::data::Base::Base(QObject *parent)
	: QObject(parent) {
	m_db = QSqlDatabase::addDatabase("QSQLITE");
	m_db.setConnectOptions("SQLITE_CONFIG_SERIALIZED");
	m_db.setDatabaseName(dp::app::Config::instance()->dataDir()  + "/media.db");
	if (!m_db.open()) qFatal("DB Open Error: %s", m_db.lastError().text().toUtf8().constData());
	initDb(m_db);
}

dp::data::Base::~Base() {
	m_db.close();
}

QVariant dp::data::Base::getFile(const QString& path) {
	QSqlQuery l_query(m_db);
	l_query.prepare("SELECT rowid, path, media, tracks, playback FROM files WHERE path = :path");
	l_query.bindValue(":path", path);
	if (!l_query.exec()) {
		Q_EMIT error(l_query.lastError().text());
	} else if (l_query.next()) {
		return QVariant::fromValue(fileFromQuery(l_query));
	}
	return QVariant();
}

QVariant dp::data::Base::createFile(const File& file) {
	QVariant l_result;
	QSqlQuery l_query(m_db);
	l_query.prepare("INSERT INTO files (path, media, tracks) VALUES (:path, :media, :tracks)");
	l_query.bindValue(":path", file.path);
	l_query.bindValue(":media", QJsonDocument::fromVariant(file.mediaMeta).toJson(QJsonDocument::Compact));
	l_query.bindValue(":tracks", QJsonDocument::fromVariant(file.tracks).toJson(QJsonDocument::Compact));
	if(!l_query.exec()) {
		qDebug() << "Error while inserting: " << file;
		Q_EMIT error(l_query.lastError().text());
	} else {
		l_result.setValue(File(
			l_query.lastInsertId().toULongLong(),
			file.path,
			file.mediaMeta,
			file.tracks,
			file.playbackData
		));
	}
	return l_result;
}

QVector<Playlist> dp::data::Base::getPlaylists() {
	QVector<Playlist> l_playlists;
	QSqlQuery l_query(m_db);
	l_query.prepare("SELECT rowid, label, files FROM playlists");
	if (!l_query.exec()) {
		Q_EMIT error(l_query.lastError().text());
	} else {
		while (l_query.next()) {
			QString l_fileList = l_query.value("files").toString();
			QVector<qulonglong> l_files;
			if (!l_fileList.isEmpty()) {
				for (const QString& l_id : l_fileList.split(',')) {
					l_files.append(l_id.toULongLong());
				}
			}
			l_playlists << Playlist(l_query.value("rowid").toUInt(), l_query.value("label").toString(), l_files);
		}
	}
	qDebug() << "Get playlists: " << l_playlists;
	return l_playlists;
}

QVector<File> dp::data::Base::getFileList(qulonglong playlistId) {
	QVector<File> l_fileList;
	QSqlQuery l_query(m_db);
	l_query.prepare("SELECT rowid, path, media, tracks, playback FROM files WHERE INSTR(',' || (SELECT files FROM playlists WHERE rowid = :id) || ',', ',' || CAST(rowid AS TEXT) || ',') > 0;");
	l_query.bindValue(":id", playlistId);
	if (!l_query.exec()) {
		Q_EMIT error(l_query.lastError().text());
	} else {
		while (l_query.next()) {
			l_fileList << fileFromQuery(l_query);
		}
	}
	qDebug() << "Fetching file list for playlist " << playlistId << " " << l_fileList;
	return l_fileList;
}

void dp::data::Base::startup() {
	QVector<Playlist> l_playlists = getPlaylists();
	if (l_playlists.isEmpty()) {
		addPlaylist("Default");
	} else {
		Q_EMIT playlistsUpdated(l_playlists);
	}
}

void dp::data::Base::updatePlaylistName(qulonglong playlistId, const QString& label) {
	QSqlQuery l_query(m_db);
	l_query.prepare("UPDATE playlists SET label = :label WHERE rowid = :id");
	l_query.bindValue(":label", label);
	l_query.bindValue(":id", playlistId);
	if(!l_query.exec()) Q_EMIT error(l_query.lastError().text());
	else Q_EMIT playlistsUpdated(getPlaylists());
}

void dp::data::Base::removePlaylist(qulonglong id) {
	QSqlQuery l_query(m_db);
	l_query.prepare("DELETE FROM playlists WHERE rowid = :id");
	l_query.bindValue(":id", id);
	if(!l_query.exec()) Q_EMIT error(l_query.lastError().text());
	else Q_EMIT playlistsUpdated(getPlaylists());
}

void dp::data::Base::addPlaylist(const QString& label) {
	QSqlQuery l_query(m_db);
	l_query.prepare("INSERT INTO playlists (label) VALUES (:label)");
	l_query.bindValue(":label", label);
	if (!l_query.exec()) Q_EMIT error(l_query.lastError().text());
	else Q_EMIT playlistsUpdated(getPlaylists());
}

void dp::data::Base::updatePlaylistFiles(qulonglong playlistId, QVector<qulonglong> files) {
	QStringList l_ids;
	std::transform(files.begin(), files.end(), std::back_inserter(l_ids), [](qulonglong id) { return QString::number(id); });	

	QSqlQuery l_query(m_db);
	l_query.prepare("UPDATE playlists SET files = :files WHERE rowid = :id");
	l_query.bindValue(":id", playlistId);
	l_query.bindValue(":files", l_ids.join(','));
	if (!l_query.exec()) Q_EMIT error(l_query.lastError().text());
	else {
		Q_EMIT playlistsUpdated(getPlaylists());
		Q_EMIT fileListUpdated(getFileList(playlistId), playlistId);
	}
}

void dp::data::Base::playlistSelected(qulonglong playlistId) {
	Q_EMIT fileListUpdated(getFileList(playlistId), playlistId);
}

void dp::data::Base::loadFiles(const QList<QString>& paths) {
	QList<File> l_result;
	for (auto& path : paths) {
		QVariant l_file = getFile(path);
		if (l_file.isValid()) {
			l_result << l_file.value<File>();
		}
	}
	Q_EMIT filesLoaded(l_result);
}

//TODO Identify files via database "path up until what? like?" and only if season and episode is set...
void dp::data::Base::createFiles(const QList<File>& files) {
	QList<File> l_result;
	for (auto& file : files) {
//		qDebug() << "Is this a series: " << file;
		QVariant l_file = createFile(file);
		if (l_file.isValid()) {
			l_result << l_file.value<File>();
		}
	}
	Q_EMIT filesCreated(l_result);	
}

QVariant dp::data::Base::getFile(qulonglong id) {
	QSqlQuery l_query(m_db);
	l_query.prepare("SELECT rowid, path, media, tracks, playback FROM files WHERE rowid = :rowid");
	l_query.bindValue(":rowid", id);
	if (!l_query.exec()) {
		Q_EMIT error(l_query.lastError().text());
	} else if (l_query.next()) {
		return QVariant::fromValue(fileFromQuery(l_query));
	}
	return QVariant();	
}


void dp::data::Base::updateTracks(qulonglong fileId, const QVariantMap& tracks) {
	QSqlQuery l_query(m_db);
	l_query.prepare("UPDATE files SET tracks = :tracks WHERE rowid = :id");
	l_query.bindValue(":tracks", QJsonDocument::fromVariant(tracks).toJson(QJsonDocument::Compact));
	l_query.bindValue(":id", fileId);
	if(!l_query.exec()) Q_EMIT error(l_query.lastError().text());
	else {
		QVariant l_file = getFile(fileId);
		if (l_file.isValid()) {
			Q_EMIT fileUpdated(l_file.value<File>());
		}
	}
}

void dp::data::Base::updatePlaybackData(qulonglong fileId, const QVariantList& playbackData) {
	QSqlQuery l_query(m_db);
	l_query.prepare("UPDATE files SET playback = :playback WHERE rowid = :id");
	l_query.bindValue(":playback", QJsonDocument::fromVariant(playbackData).toJson(QJsonDocument::Compact));
	l_query.bindValue(":id", fileId);
	if(!l_query.exec()) Q_EMIT error(l_query.lastError().text());
	else {
		QVariant l_file = getFile(fileId);
		if (l_file.isValid()) {
			Q_EMIT fileUpdated(l_file.value<File>());
		}
	}	
}

void dp::data::Base::updateMetaData(qulonglong fileId, const QVariantMap& mediaMeta) {
	QSqlQuery l_query(m_db);
	l_query.prepare("UPDATE files SET media = :media WHERE rowid = :id");
	l_query.bindValue(":media", QJsonDocument::fromVariant(mediaMeta).toJson(QJsonDocument::Compact));
	l_query.bindValue(":id", fileId);
	if(!l_query.exec()) Q_EMIT error(l_query.lastError().text());
	else {
		QVariant l_file = getFile(fileId);
		if (l_file.isValid()) {
			Q_EMIT fileUpdated(l_file.value<File>());
		}
	}
}
