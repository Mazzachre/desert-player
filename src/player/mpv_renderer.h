#pragma once

#include <QGuiApplication>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QQuickWindow>

#include <QVariant>
#include <QString>
#include <QList>
#include <QHash>
#include <QMap>
#include <QSharedPointer>
#include <QMetaType>

#include <mpv/client.h>
#include "mpv_player.h"

namespace dp {
namespace player {

class MpvRenderer : public QQuickFramebufferObject::Renderer
{
private:
	MpvPlayer *m_player;
public:
	explicit MpvRenderer(MpvPlayer *player);
	virtual ~MpvRenderer() = default;
	QOpenGLFramebufferObject* createFramebufferObject(const QSize &size) override;
	void render() override;
};

}}
