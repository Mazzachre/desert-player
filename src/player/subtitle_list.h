#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <QPointer>
#include <QUrl>

namespace dp {
namespace app {class App;}
namespace player {

class SubtitleList : public QAbstractListModel {
	Q_OBJECT
    Q_PROPERTY(QUrl directory READ getDirectory NOTIFY directoryChanged)
    Q_PROPERTY(QUrl fileName READ getFileName NOTIFY fileNameChanged)
    Q_PROPERTY(bool external READ isExternal NOTIFY externalChanged)

public:
	static SubtitleList* instance();

	enum SubtitleTrackRoles {
		IdRole = Qt::UserRole + 1,
		SelectedRole,
		LabelRole,
		ExternalRole,
		FilenameRole
	};

	QHash<int, QByteArray> roleNames() const override;
	int rowCount(const QModelIndex&) const override;
	QVariant data(const QModelIndex& index, int role) const override;

	QUrl getDirectory() const;
	Q_SIGNAL void directoryChanged() const;

	QString getFileName() const;
	Q_SIGNAL void fileNameChanged() const;

	bool isExternal() const;
	Q_SIGNAL void externalChanged() const;

	Q_SLOT void removeSubtitle();
    Q_SIGNAL void removeSubtitleTrack(const QVariant& trackId);

private:
	static QPointer<SubtitleList> m_instance;
	explicit SubtitleList(QObject* parent = nullptr);
	static void init(QObject* parent = nullptr);

	QVector<QHash<int, QVariant>> m_backing;
    QVariant m_selected;
	QUrl m_directory;
	QString m_fileName;
	bool m_external;

	Q_SLOT void setTracks(const QList<QVariant>& tracks);
    Q_SLOT void setSelected(const QVariant& trackId);
    Q_SLOT void movieStarted(const QString& file);

	friend class dp::app::App;
};
}}
