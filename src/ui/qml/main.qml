import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.3
import QtQuick.Layouts 1.11
import desert 1.0

Window {
	id: mainWindow
	title: "Desert Player"
	visible: true

	FontLoader {
		id: icons
		source: "qrc:/icons/Icons.ttf"
	}

	Library {
		visible: WindowController.currentState == WindowController.Library
		anchors.fill: parent
	}

	Player {
		visible: WindowController.currentState == WindowController.Player
		anchors.fill: parent
	}
}
