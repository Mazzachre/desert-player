#pragma once

#include <QQuickWindow>
#include <QEvent>
#include <QPointer>
#include <QTimer>
#include <QX11Info>
#include "../data/types.h"

namespace dp {
namespace app {class App;}
namespace ui {

class WindowController : public QObject {
	Q_OBJECT
	Q_PROPERTY(bool controlsVisible READ isControlsVisible NOTIFY controlsVisisbleChanged)
	Q_PROPERTY(State currentState READ currentState NOTIFY stateChanged)
	Q_PROPERTY(bool fullScreen READ isFullScreen NOTIFY fullScreenChanged)

public:
	static WindowController* instance();

	enum State {
		Library,
		Player
	};
	Q_ENUM(State)

	Q_SLOT void startPlaying();
	Q_SLOT void stopPlaying();

	Q_SLOT void toggleFullScreen();
	bool isFullScreen();
	Q_SIGNAL void fullScreenChanged();

	bool isControlsVisible();
	Q_SIGNAL void controlsVisisbleChanged();

	State currentState();
	Q_SIGNAL void stateChanged();
	
	Q_SIGNAL void playerTogglePause();
	Q_SIGNAL void playerToggleSubtitles();
	Q_SIGNAL void playerSeek(int amount);
	
private:
	static QPointer<WindowController> m_instance;
	static void init(QObject* parent = nullptr);
	WindowController(QObject* parent = nullptr);
	void init(QQuickWindow* window, QRect screenSize);
	
	QQuickWindow* m_window{nullptr};
	QTimer* m_controlsTimer{nullptr};
	Display* m_display{nullptr};
	bool m_controlsVisible = true;
	State m_state = Library;

    bool eventFilter(QObject* watched, QEvent* event) override;

    Q_SLOT void hideControls();

	friend class dp::app::App;
};
}}
