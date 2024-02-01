#pragma once

#include <QPointer>
#include "../data/types.h"

namespace dp {
namespace app {class App;}
namespace library {

class Playback : public QObject {
	Q_OBJECT

public:
	static Playback* instance();
	
	Q_SLOT void startPlayback(const File& file);
	Q_SLOT void stopPlayback();
	Q_SLOT void videoTrackSelected(const QVariant& trackId);
	Q_SLOT void audioTrackSelected(const QVariant& trackId);
	Q_SLOT void subtitleTrackSelected(const QVariant& trackId);
	Q_SLOT void subtitleTrackRemoved(const QVariant& trackId);
	Q_SLOT void volumeChanged(uint volume);
	Q_SLOT void positionChanged(qulonglong position);

	Q_SIGNAL void tracksUpdated(const QString& path, const QVariantMap& tracks, uint playlistId);
	Q_SIGNAL void playbackFinished(const QString& path, const QVariantList& playbackData, uint playlistId);

private:
	static QPointer<Playback> m_instance;
	explicit Playback(QObject* parent = nullptr);
	~Playback();
	static void init(QObject* parent = nullptr);

	File m_file;
	qint64 m_started = 0;
	uint m_accumulated = 0;
	uint m_position = 0;

	friend class dp::app::App;
};
}}
