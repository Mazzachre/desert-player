#include "data.h"
#include "../app/config.h"
#include <QSqlQuery>
#include <QSqlError>
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
	if (script.isEmpty()) qFatal("Unable to run database script %d", version);
	QSqlQuery query(db);
	for (auto& line : script.split("\n")) {
		if (line.trimmed().size() > 0) {
			if (!query.exec(line)) qFatal("Sql Script Error: %s", db.lastError().text().toUtf8().constData());
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
		entry.value("path").toString(),
		QJsonDocument::fromJson(entry.value("media").toByteArray()).object().toVariantMap(),
		QJsonDocument::fromJson(entry.value("tracks").toByteArray()).object().toVariantMap(),
		QJsonDocument::fromJson(entry.value("playback").toByteArray()).array().toVariantList()
	);
}

QPointer<dp::data::Data> dp::data::Data::m_instance = nullptr;

dp::data::Data* dp::data::Data::instance() {
	if (!m_instance) qFatal("Database accessed before initialized");
	return m_instance;
}

dp::data::Data::Data(QObject *parent)
	: QObject(parent) {
	m_db = QSqlDatabase::addDatabase("QSQLITE");
	m_db.setDatabaseName(dp::app::Config::instance()->dataDir()  + "/media.db");
	if (!m_db.open()) qFatal("DB Open Error: %s", m_db.lastError().text().toUtf8().constData());
	initDb(m_db);
}

dp::data::Data::~Data() {
	m_db.close();
}

void dp::data::Data::init(QObject *parent) {
	if (!m_instance) m_instance = new Data(parent);
}

QVariant dp::data::Data::getFile(const QString& path) {
	QSqlQuery l_query(m_db);
	l_query.prepare("SELECT path, media, tracks, playback FROM files WHERE path = :path");
	l_query.bindValue(":path", path);
	if (!l_query.exec()) {
		Q_EMIT error(l_query.lastError().text());
	} else if (l_query.next()) {
		return QVariant::fromValue(fileFromQuery(l_query));
	}
	return QVariant();
}

QVariant dp::data::Data::createFile(const File& file) {
	QVariant l_file;
	QSqlQuery l_query(m_db);
	l_query.prepare("INSERT INTO files (path, media, tracks) VALUES (:path, :media, :tracks)");
	l_query.bindValue(":path", file.path);
	l_query.bindValue(":media", QJsonDocument::fromVariant(file.mediaMeta).toJson(QJsonDocument::Compact));
	l_query.bindValue(":tracks", QJsonDocument::fromVariant(file.tracks).toJson(QJsonDocument::Compact));
	if(!l_query.exec()) {
		Q_EMIT error(l_query.lastError().text());
	} else {
		l_file.setValue(file);
	}
	return l_file;
}

QVector<Playlist> dp::data::Data::getPlaylists() {
	QVector<Playlist> l_playlists;
	QSqlQuery l_query(m_db);
	l_query.prepare("SELECT rowid, label FROM playlists");
	if (!l_query.exec()) {
		Q_EMIT error(l_query.lastError().text());
	} else {
		while (l_query.next()) {
			l_playlists << Playlist(l_query.value("rowid").toUInt(), l_query.value("label").toString());
		}
	}
	return l_playlists;
}

QVector<File> dp::data::Data::getFileList(uint playlistId) {
	QVector<File> l_fileList;
	QSqlQuery l_query(m_db);
	l_query.prepare("SELECT path, media, tracks, playback FROM files WHERE path IN (SELECT file FROM playlistFiles WHERE playlist = :id) ORDER BY rowid");
	l_query.bindValue(":id", playlistId);
	if (!l_query.exec()) {
		Q_EMIT error(l_query.lastError().text());
	} else {
		while (l_query.next()) {
			l_fileList << fileFromQuery(l_query);
		}
	}
	return l_fileList;
}

void dp::data::Data::startup() {
	QVector<Playlist> l_playlists = getPlaylists();
	if (l_playlists.isEmpty()) {
		addPlaylist("Default");
	} else {
		Q_EMIT playlistsUpdated(l_playlists);
	}
}

void dp::data::Data::updatePlaylistName(uint playlistId, const QString& label) {
	QSqlQuery l_query(m_db);
	l_query.prepare("UPDATE playlists SET label = :label WHERE rowid = :id");
	l_query.bindValue(":label", label);
	l_query.bindValue(":id", playlistId);
	if(!l_query.exec()) Q_EMIT error(l_query.lastError().text());
	else Q_EMIT fileListUpdated(getFileList(playlistId), playlistId);
}

void dp::data::Data::removePlaylist(uint id) {
	QSqlQuery l_pl_query(m_db);
	l_pl_query.prepare("DELETE FROM playlists WHERE rowid = :id");
	l_pl_query.bindValue(":id", id);
	if(l_pl_query.exec()) {
		QSqlQuery l_fl_query(m_db);
		l_fl_query.prepare("DELETE FROM playlistFiles WHERE playlist = :id");
		l_fl_query.bindValue(":id", id);
		if(!l_fl_query.exec()) Q_EMIT error(l_pl_query.lastError().text());
		else Q_EMIT playlistsUpdated(getPlaylists());
	}
	Q_EMIT error(l_pl_query.lastError().text());
}

void dp::data::Data::addPlaylist(const QString& label) {
	QSqlQuery l_query(m_db);
	l_query.prepare("INSERT INTO playlists (label) VALUES (:label)");
	l_query.bindValue(":label", label);
	if (!l_query.exec()) Q_EMIT error(l_query.lastError().text());
	else Q_EMIT playlistsUpdated(getPlaylists());
}

void dp::data::Data::removeFileFromPlaylist(const QString& file, uint playlistId) {
	QSqlQuery l_query(m_db);
	l_query.prepare("DELETE FROM playlistFiles WHERE playlist = :id AND file = :file");
	l_query.bindValue(":id", playlistId);
	l_query.bindValue(":file", file);
	if(!l_query.exec()) Q_EMIT error(l_query.lastError().text());
	else Q_EMIT fileListUpdated(getFileList(playlistId), playlistId);
}

void dp::data::Data::addFilesToPlaylist(const QList<File>& files, uint playlistId) {
	QSqlQuery l_query(m_db);
	l_query.prepare("INSERT INTO playlistFiles (playlist, file) VALUES (:playlist, :file)");
	QVariantList idList;
	QVariantList fileList;
	for (auto& file : files) {
		idList << playlistId;
		fileList << file.path;
	}
	l_query.bindValue(":playlist", idList);
	l_query.bindValue(":file", fileList);
	if (!l_query.execBatch()) Q_EMIT error(l_query.lastError().text());
	else Q_EMIT fileListUpdated(getFileList(playlistId), playlistId);
}

void dp::data::Data::playlistSelected(uint playlistId) {
	Q_EMIT fileListUpdated(getFileList(playlistId), playlistId);
}

void dp::data::Data::loadFiles(const QList<QString>& paths) {
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
void dp::data::Data::createFiles(const QList<File>& files) {
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

void dp::data::Data::updateTracks(const QString& path, const QVariantMap& tracks, uint playlistId) {
	QSqlQuery l_query(m_db);
	l_query.prepare("UPDATE files SET tracks = :tracks WHERE path = :path");
	l_query.bindValue(":tracks", QJsonDocument::fromVariant(tracks).toJson(QJsonDocument::Compact));
	l_query.bindValue(":path", path);
	if(!l_query.exec()) Q_EMIT error(l_query.lastError().text());
	else Q_EMIT fileListUpdated(getFileList(playlistId), playlistId);
}

void dp::data::Data::updatePlaybackData(const QString& path, const QVariantList& playbackData, uint playlistId) {
	QSqlQuery l_query(m_db);
	l_query.prepare("UPDATE files SET playback = :playback WHERE path = :path");
	l_query.bindValue(":playback", QJsonDocument::fromVariant(playbackData).toJson(QJsonDocument::Compact));
	l_query.bindValue(":path", path);
	if(!l_query.exec()) Q_EMIT error(l_query.lastError().text());
	else Q_EMIT fileListUpdated(getFileList(playlistId), playlistId);
}
