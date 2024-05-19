#pragma once

#include <QPointer>
#include <QFile>
#include <QMutex>
#include <QString>
#include <QtDebug>

namespace dp {
namespace app {

class Logger : public QObject {
	Q_OBJECT

public:
    static Logger* instance();
    void startLogging();
    void stopLogging();
    static void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    Logger(const QString& fileName, QObject *parent = nullptr);
    ~Logger();
	static void init(QObject *parent = nullptr);

	static QPointer<Logger> m_instance;
    QFile m_logFile;
    QMutex m_logMutex;

	friend class App;
};

}}
