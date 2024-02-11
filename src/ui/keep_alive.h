#pragma once

#include <QX11Info>
#include <QObject>

namespace dp {
namespace ui {

class KeepAlive : public QObject {
	Q_OBJECT
public:
	KeepAlive(QObject* parent = nullptr);
	Q_SLOT void moviePlaying();
private:
	Display* m_display{nullptr};
};
}}
