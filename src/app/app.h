#pragma once

#include <QApplication>
#include <QQmlApplicationEngine>
#include "../ui/keep_alive.h"

namespace dp {
namespace app {

class App : public QApplication {
	Q_OBJECT

public:
	App(int &argc, char **argv);
	~App();

	int run();

private:
	QQmlApplicationEngine *m_engine;
	dp::ui::KeepAlive *m_keepAlive;

	void registerQmlTypes();
	void connectSignals();

	Q_SLOT void handleError(const QString& error);
};
}}
