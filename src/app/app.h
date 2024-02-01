#pragma once

#include <QApplication>
#include <QQmlApplicationEngine>

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

	void registerQmlTypes();
	void connectSignals();

	Q_SLOT void handleError(const QString& error);
};
}}
