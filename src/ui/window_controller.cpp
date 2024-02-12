#include "window_controller.h"
#include <QKeyEvent>

QPointer<dp::ui::WindowController> dp::ui::WindowController::m_instance = nullptr;

dp::ui::WindowController* dp::ui::WindowController::instance() {
	if (!m_instance) qFatal("Window Controller accessed before initialized");
	return m_instance;
}

dp::ui::WindowController::WindowController(QObject* parent)
	: QObject(parent)
	, m_controlsTimer{new QTimer(this)}
	, m_display{QX11Info::display()} {
	connect(m_controlsTimer, &QTimer::timeout, this, &WindowController::hideControls);
}

void dp::ui::WindowController::init(QObject *parent) {
	if (!m_instance) m_instance = new WindowController(parent);
}

void dp::ui::WindowController::init(QQuickWindow* window, QRect screenSize) {
	m_window = window;
	m_window->resize(screenSize.width() / 2, screenSize.height() / 2);
	m_window->installEventFilter(this);
}

void dp::ui::WindowController::hideControls() {
	m_controlsTimer->stop();
	m_controlsVisible = false;
	Q_EMIT controlsVisisbleChanged();
}

bool dp::ui::WindowController::isControlsVisible() {
	return m_controlsVisible;
}

void dp::ui::WindowController::startPlaying() {
	m_state = Player;
	Q_EMIT stateChanged();
	m_controlsTimer->start(5000);
}

void dp::ui::WindowController::stopPlaying() {
	if (m_window != nullptr) {
		m_window->setWindowState(Qt::WindowNoState);
	}

	m_controlsTimer->stop();
	m_controlsVisible = true;
	Q_EMIT controlsVisisbleChanged();

	m_state = Library;
	Q_EMIT stateChanged();
}

void dp::ui::WindowController::toggleFullScreen() {
	if (m_window != nullptr) {
		if (m_window->windowState() == Qt::WindowNoState) {
			m_window->setWindowState(Qt::WindowFullScreen);
		} else {
			m_window->setWindowState(Qt::WindowNoState);
		}
		Q_EMIT fullScreenChanged();
	}
}

bool dp::ui::WindowController::isFullScreen() {
	return m_window != nullptr && m_window->windowState() == Qt::WindowFullScreen;
}

dp::ui::WindowController::State dp::ui::WindowController::currentState() {
	return m_state;
}

bool dp::ui::WindowController::eventFilter(QObject* watched, QEvent* event) {
	if (m_state == Player) {
		if (event->type() == QEvent::MouseMove) {
			m_controlsTimer->start(5000);
			if (!m_controlsVisible) {
				m_controlsVisible = true;
				Q_EMIT controlsVisisbleChanged();
			}
		}
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
			if (keyEvent->key() == Qt::Key_S) {
				Q_EMIT playerToggleSubtitles();
			}
			if (keyEvent->key() == Qt::Key_Space) {
				Q_EMIT playerTogglePause();
			}
			if (keyEvent->key() == Qt::Key_Right) {
				Q_EMIT playerSeek(5);
			}
			if (keyEvent->key() == Qt::Key_Left) {
				Q_EMIT playerSeek(-5);
			}
			event->accept();
			return true;
		}
	}
	return QObject::eventFilter(watched, event);
}
