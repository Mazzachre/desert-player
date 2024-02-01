#include "app/app.h"

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QCoreApplication::setOrganizationName(QStringLiteral("Real Desert"));
	QCoreApplication::setOrganizationDomain(QStringLiteral("real-desert.com"));
	QCoreApplication::setApplicationName(QStringLiteral("Desert Player"));

	dp::app::App app(argc, argv);
	return app.run();
}
