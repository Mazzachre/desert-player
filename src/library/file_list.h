#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <QPointer>
#include "../data/types.h"

namespace dp {
namespace app {class App;}
namespace library {

class FileList : public QAbstractListModel
{
	Q_OBJECT
	Q_PROPERTY(bool playable READ getPlayable NOTIFY playableUpdated)
	Q_PROPERTY(bool hasNext READ hasNext NOTIFY fileSelected)
	Q_PROPERTY(bool hasPrev READ hasPrev NOTIFY fileSelected)

public:
	static FileList* instance();

	enum PlaylistsRoles {
		PathRole = Qt::UserRole + 1,
		TitleRole,
		DurationRole,
		HasSubtitleRole,
		SelectedRole
	};

	QHash<int, QByteArray> roleNames() const override;
	int rowCount(const QModelIndex&) const override;
	QVariant data(const QModelIndex &index, int role) const override;

	Q_SLOT void selectFile(const QString& path);
	Q_SIGNAL void fileSelected(const QString& path);
	
	Q_SLOT void startPlaying();
	Q_SLOT void playNext();
	Q_SLOT void playPrev();
	Q_SIGNAL void playFile(const File& file);
	Q_SIGNAL void playlistFinished();
	
	bool getPlayable();
	Q_SIGNAL void playableUpdated();

	bool hasNext();
	bool hasPrev();

private:
	static QPointer<FileList> m_instance;
	explicit FileList(QObject* parent = nullptr);
	static void init(QObject* parent = nullptr);

	QVector<File> m_backing;
	QString m_selected;
	uint m_playlistId;

	Q_SLOT void setFileList(const QVector<File>& files, uint playlistId);

	friend class dp::app::App;	
};
}}
