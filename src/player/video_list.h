#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <QPointer>
#include <QTimer>
#include "../data/types.h"

namespace dp {
namespace app {class App;}
namespace player {

class VideoList : public QAbstractListModel
{
    Q_OBJECT
	Q_PROPERTY(bool paused READ getPaused NOTIFY pausedChanged)
    Q_PROPERTY(qulonglong duration READ getDuration NOTIFY durationChanged)
    Q_PROPERTY(qulonglong position READ getPosition NOTIFY positionChanged)
    Q_PROPERTY(QVariantMap continueData READ continueData NOTIFY continueDataChanged)

public:
	static VideoList* instance();

    enum VideoTrackRoles {
        IdRole = Qt::UserRole + 1,
        CodecRole,
        LabelRole,
        SelectedRole,
    };

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex &index, int role) const override;

	bool getPaused() const;
	Q_SIGNAL void pausedChanged() const;
	qulonglong getDuration() const;
    Q_SIGNAL void durationChanged() const;
	qulonglong getPosition() const;
    Q_SIGNAL void positionChanged() const;

	Q_SLOT void changePosition(qulonglong position) const;
	Q_SIGNAL void positionChange(qulonglong position) const;
	
	QVariantMap continueData();
	Q_SLOT void continuePlayback();
	Q_SIGNAL void continueDataChanged();

private:
	static QPointer<VideoList> m_instance;
	explicit VideoList(QObject *parent = nullptr);
	static void init(QObject *parent = nullptr);

	qulonglong m_duration = 0;
	qulonglong m_position = 0;
	bool m_paused = false;
	QVector<QHash<int, QVariant>> m_backing;
	QVariant m_selected;
	QVariantMap m_continueData = {{"show", false}};
	QTimer* m_timer;

	Q_SLOT void setTracks(const QList<QVariant>& tracks);
	Q_SLOT void setSelected(const QVariant& trackId);
	Q_SLOT void setPaused(bool paused);
	Q_SLOT void setDuration(qulonglong duration);
	Q_SLOT void setPosition(qulonglong position);
	
	Q_SLOT void startPlayback(const File& file);
	Q_SLOT void hideContinue();

	friend class dp::app::App;
};
}}
