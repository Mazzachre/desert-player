#include "keep_alive.h"
extern "C" {
#include <X11/Xlib.h>
}

dp::ui::KeepAlive::KeepAlive(QObject* parent)
	: QObject(parent)
	, m_display{QX11Info::display()} {}

void dp::ui::KeepAlive::moviePlaying() {
	XResetScreenSaver(m_display);
}
