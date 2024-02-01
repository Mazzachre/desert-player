#include "config.h"

#include <QStandardPaths>
#include <QDir>
#include <QDebug>

QPointer<dp::app::Config> dp::app::Config::m_instance = nullptr;

dp::app::Config* dp::app::Config::instance() {
	if (!m_instance) qFatal("Config accessed before initialized");
	return m_instance;
}

dp::app::Config::Config(QObject *parent)
	: QObject(parent) {
	QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
	//Here I can load the actual config...
}

void dp::app::Config::init(QObject *parent) {
	m_instance = new Config(parent);
}

uint dp::app::Config::version() const {
	return 1;
}

QString dp::app::Config::dataDir() const {
	return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString dp::app::Config::cacheDir() const {
	return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
}

//User config...

bool dp::app::Config::debug() const {
	return true;
}

uint dp::app::Config::cacheSize() const {
	return m_values.value("cacheSize", 256 * 1024 * 1024).toULongLong();
}

QString dp::app::Config::language(LanguageCodes code) const {
	switch(code) {
		case LanguageCodes::ISO_639_2: return "eng";
		case LanguageCodes::ISO_3166_1: return "en-US";
		default: return "unknown";
	}
}

QString dp::app::Config::dateFormat() const {
	return m_values.value("dateFormat", "MM/dd/yyyy").toString();
}

QString dp::app::Config::timeFormat() const {
	return m_values.value("timeFormat", "h:mm ap").toString();
}

uint dp::app::Config::watchedBeforeSave() const {
	return m_values.value("watchedBeforeSave", 300).toULongLong();
}

uint dp::app::Config::showContinueFor() const {
	return m_values.value("showContinueFor", 15000).toULongLong();
}

uint dp::app::Config::showContinueBefore() const {
	return m_values.value("showContinueBefore", 14* 24 * 60 * 60).toULongLong();
}
