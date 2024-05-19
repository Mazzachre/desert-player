#include "logger.h"
#include "config.h"
#include <QTextStream>
#include <QDateTime>

QPointer<dp::app::Logger> dp::app::Logger::m_instance = nullptr;

dp::app::Logger* dp::app::Logger::instance() {
	if (!m_instance) qFatal("Logger accessed before initialized");
    return m_instance;
}

void dp::app::Logger::init(QObject *parent) {
	m_instance = new Logger(Config::instance()->logFile(), parent);
}

dp::app::Logger::Logger(const QString &fileName, QObject *parent)
	: QObject(parent), m_logFile(fileName) {
}

void dp::app::Logger::startLogging() {
    QMutexLocker locker(&m_logMutex);
    if (!m_logFile.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Unable to open log file for writing.";
        return;
    }
    qInstallMessageHandler(customMessageHandler);
}

void dp::app::Logger::stopLogging() {
    QMutexLocker locker(&m_logMutex);
    qInstallMessageHandler(nullptr);
    if (m_logFile.isOpen()) {
        m_logFile.flush();
        m_logFile.close();
    }
}

dp::app::Logger::~Logger() {
    stopLogging();
}

void dp::app::Logger::customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Logger *logger = Logger::instance();
    QMutexLocker locker(&logger->m_logMutex);

    QString logLevel;
    switch (type) {
        case QtDebugMsg: logLevel = "DEBUG"; break;
        case QtInfoMsg: logLevel = "INFO"; break;
        case QtWarningMsg: logLevel = "WARNING"; break;
        case QtCriticalMsg: logLevel = "CRITICAL"; break;
        case QtFatalMsg: logLevel = "FATAL"; break;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logMessage = QString("%1 [%2] (%3:%4, %5) %6")
                            .arg(timestamp)
                            .arg(logLevel)
                            .arg(context.file)
                            .arg(context.line)
                            .arg(context.function)
                            .arg(msg);

    QTextStream out(&logger->m_logFile);
    out << logMessage << ::endl;
    if (type == QtFatalMsg) {
        abort();
    }
}
