#pragma once

#include <QVariant>
#include <QString>
#include <QMetaType>

#include <mpv/client.h>

struct MpvErrorType {
	QVariant error;
	MpvErrorType() {};
	explicit MpvErrorType(const QString &_error): error(_error) {};
	bool isValid() { return error.isValid(); };
	QString toString() { return error.toString(); };
};
Q_DECLARE_METATYPE(MpvErrorType);

namespace dp {
namespace player {

class MpvController : public QObject
{
	Q_OBJECT

public:
	MpvController(bool verbose, QObject *parent = nullptr);

	Q_INVOKABLE QVariant setProperty(const QString &property, const QVariant &value);
	Q_INVOKABLE QVariant getProperty(const QString &property);
	Q_INVOKABLE QVariant command(const QVariant &params);
	Q_INVOKABLE void observeProperty(const QString &property, const mpv_format format);

	mpv_handle *mpv() const;

	Q_SIGNAL void propertyChanged(const QString &property, const QVariant &value);
	Q_SIGNAL void tracksLoaded();
	Q_SIGNAL void endFile();
	Q_SIGNAL void endError(const QString& error);

private:
	mpv_handle *m_mpv{nullptr};

	static void mpvEvents(void *ctx);
	void eventHandler();
};
}}
