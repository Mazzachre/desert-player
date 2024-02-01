#pragma once

#include <QtQuick/QQuickFramebufferObject>
#include <mpv/client.h>
#include <mpv/render_gl.h>
#include "mpv_controller.h"
#include "../data/types.h"

namespace dp {
namespace player {

class MpvPlayer : public QQuickFramebufferObject {
	Q_OBJECT
	
public:
	MpvPlayer(QQuickItem *parent = nullptr);
	~MpvPlayer();

	Renderer *createRenderer() const override;

	Q_SIGNAL void error(const QString& error) const;
	
	Q_SLOT void togglePause() const;
	Q_SLOT void setPause(bool pause) const;
	Q_SIGNAL void pausedChanged(bool paused) const;
	Q_SIGNAL void durationChanged(qulonglong duration) const;
	Q_SLOT void changePosition(const QVariant& position) const;
	Q_SIGNAL void positionChanged(qulonglong position) const;
	Q_SLOT void seekPosition(qlonglong amount) const;

	Q_SLOT void changeVolume(const QVariant& volume) const;
	Q_SIGNAL void volumeChanged(uint volume) const;
	Q_SLOT void toggleMute() const;
	Q_SIGNAL void mutedChanged(bool muted) const;

	Q_SLOT void changeVideoTrack(const QVariant& trackId) const;
	Q_SIGNAL void videoTrackChanged(const QVariant& track) const;
	Q_SLOT void changeAudioTrack(const QVariant& trackId) const;
	Q_SIGNAL void audioTrackChanged(const QVariant& track) const;
	Q_SLOT void changeSubtitleTrack(const QVariant& subtitle) const;
	Q_SLOT void removeSubtitleTrack(const QVariant& subtitle) const;
	Q_SIGNAL void subtitleTrackRemoved(const QVariant& subtitle) const;
	Q_SIGNAL void subtitleTrackChanged(const QVariant& subtitle) const;

	Q_SLOT void playMovie(const QMap<QString, QVariant>& args);
	Q_SLOT void playFile(const File& file);
	Q_SLOT void stopMovie() const;

	Q_SIGNAL void movieStarted(const QString& file) const;
	Q_SIGNAL void movieStopped() const;
	Q_SIGNAL void movieCancelled() const;

	Q_SIGNAL void videoTracksLoaded(const QList<QVariant>& tracks) const;
	Q_SIGNAL void audioTracksLoaded(const QList<QVariant>& tracks) const;
	Q_SIGNAL void subtitleTracksLoaded(const QList<QVariant>& tracks) const;

	Q_SLOT void toggleSubtitles() const;
	Q_SIGNAL void subtitlesVisibleChanged(bool visible) const;

private:
	QThread* m_controllerThread{nullptr};
	MpvController* m_mpvController{nullptr};
	mpv_handle* m_mpv{nullptr};
	mpv_render_context* m_mpv_gl{nullptr};
	QMap<QString, QVariant> m_movie;
	QMap<uint, QString> m_subtitle_files;

	Q_SLOT void handlePropertyChange(const QString &property, const QVariant &value);
	Q_SLOT void handleTracksLoaded() const;
	Q_SLOT void handleFileEnd() const;
	Q_SLOT void handleFileError(const QString& reason) const;

	friend class MpvRenderer;

};
}}
