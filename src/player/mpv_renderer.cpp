#include "mpv_renderer.h"

#include <stdexcept>
#include <clocale>

#include <QDebug>

#include <QObject>
#include <QtGlobal>
#include <QOpenGLContext>
#include <QGuiApplication>

#include <QtQuick/QQuickWindow>
#include <QtQuick/QQuickView>

static void *get_proc_address_mpv(void *ctx, const char *name)
{
	Q_UNUSED(ctx)

	QOpenGLContext *glctx = QOpenGLContext::currentContext();
	if (!glctx) {
		return nullptr;
	}

	return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}

void on_mpv_redraw(void *ctx)
{
	QMetaObject::invokeMethod(static_cast<dp::player::MpvPlayer *>(ctx), &dp::player::MpvPlayer::update, Qt::QueuedConnection);
}

dp::player::MpvRenderer::MpvRenderer(MpvPlayer *player): m_player{player} {
	m_player->window()->setPersistentOpenGLContext(true);
}

QOpenGLFramebufferObject* dp::player::MpvRenderer::createFramebufferObject(const QSize &size) {
	if (!m_player->m_mpv_gl) {
		mpv_opengl_init_params gl_init_params{get_proc_address_mpv, nullptr, nullptr};

		mpv_render_param display{MPV_RENDER_PARAM_INVALID, nullptr};
		mpv_render_param params[]{
			{MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
			{MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
			display,
			{MPV_RENDER_PARAM_INVALID, nullptr}
		};

		int result = mpv_render_context_create(&m_player->m_mpv_gl, m_player->m_mpv, params);
		if (result < 0) {
			qFatal("failed to initialize mpv GL context");
		}

		mpv_render_context_set_update_callback(m_player->m_mpv_gl, on_mpv_redraw, m_player);
	}

	return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
}

void dp::player::MpvRenderer::render() {
	if (m_player->window() != nullptr) {
		m_player->window()->resetOpenGLState();
	}

	QOpenGLFramebufferObject *fbo = framebufferObject();
	mpv_opengl_fbo mpfbo;
	mpfbo.fbo = static_cast<int>(fbo->handle());
	mpfbo.w = fbo->width();
	mpfbo.h = fbo->height();
	mpfbo.internal_format = 0;

	int flip_y{0};

	mpv_render_param params[] = {{MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
		{MPV_RENDER_PARAM_FLIP_Y, &flip_y},
		{MPV_RENDER_PARAM_INVALID, nullptr}};
	mpv_render_context_render(m_player->m_mpv_gl, params);

	if (m_player->window() != nullptr) {
		m_player->window()->resetOpenGLState();
	}
}
