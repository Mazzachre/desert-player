#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <QPointer>
#include "../data/types.h"

namespace dp {
namespace app {class App;}
namespace library {

class Playlists : public QAbstractListModel
{
	Q_OBJECT
	Q_PROPERTY(QString label READ getLabel NOTIFY playlistSelected);

public:
	static Playlists* instance();

	enum PlaylistsRoles {
		IdRole = Qt::UserRole + 1,
		SelectedRole,
		LabelRole
	};

	QHash<int, QByteArray> roleNames() const override;
	int rowCount(const QModelIndex&) const override;
	QVariant data(const QModelIndex &index, int role) const override;

	Q_SLOT void selectPlaylist(qulonglong id);
	Q_SIGNAL void playlistSelected(qulonglong id);

	qulonglong getSelected();
	QString getLabel();
	QVector<qulonglong> getFiles();

private:
	static QPointer<Playlists> m_instance;
	explicit Playlists(QObject *parent = nullptr);
	static void init(QObject *parent = nullptr);

	qulonglong m_selected = 0;
	QVector<Playlist> m_backing;

	Q_SLOT void setPlaylists(const QVector<Playlist>& playlists);

	friend class dp::app::App;
};
}}
