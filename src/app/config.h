#pragma once

#include <QPointer>
#include <QMap>


namespace dp {
namespace app {

class Config : public QObject {
	Q_OBJECT

public:
	static Config* instance();

	enum class LanguageCodes {
		ISO_639,
		ISO_639_2,
		ISO_3166_1
	};

	uint version() const;
	bool debug() const;
	QString logFile() const;
	QString dataDir() const;
	QString cacheDir() const;
	uint cacheSize() const;
	QString language(LanguageCodes code) const;
	QString dateFormat() const;
	QString timeFormat() const;
	uint watchedBeforeSave() const;
	uint ignoreContinueAfter() const;
	uint showContinueFor() const;
	uint showContinueBefore() const;

private:
	static QPointer<Config> m_instance;
	Config(QObject *parent = nullptr);
	static void init(QObject *parent = nullptr);

	QVariantMap m_values;

	friend class App;
};
	
}}
