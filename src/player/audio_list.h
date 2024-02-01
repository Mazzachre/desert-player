#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <QPointer>

namespace dp {
namespace app {class App;}
namespace player {

class AudioList : public QAbstractListModel {
    Q_OBJECT
	Q_PROPERTY(bool muted READ getMuted NOTIFY mutedChanged)
    Q_PROPERTY(uint volume READ getVolume NOTIFY volumeChanged)
	
public:
	static AudioList* instance();

    enum AudioTrackRoles {
        IdRole = Qt::UserRole + 1,
        SelectedRole,
        LabelRole,
        CodecRole,
        ChannelsRole,
    };

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex& index, int role) const override;

	bool getMuted() const;
	Q_SIGNAL void mutedChanged() const;
	uint getVolume() const;
    Q_SIGNAL void volumeChanged() const;

	Q_SLOT void changeVolume(uint volume) const;
	Q_SIGNAL void volumeChange(uint volume) const;

private:
	static QPointer<AudioList> m_instance;
	explicit AudioList(QObject* parent = nullptr);
	static void init(QObject* parent = nullptr);

	QVector<QHash<int, QVariant>> m_backing;
	QVariant m_selected;
	uint m_volume = 100;
	bool m_muted;

	Q_SLOT void setMuted(bool muted);
	Q_SLOT void setVolume(uint volume);

	Q_SLOT void setTracks(const QList<QVariant>& tracks);
	Q_SLOT void setSelected(const QVariant& trackId);

	friend class dp::app::App;
};
}}
