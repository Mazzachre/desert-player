import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11
import desert 1.0

Item {
	anchors.fill: parent

	Menu {
		id: playerMenu
		AudioMenu {}
		VideoMenu {}
		SubtitleMenu {}
	}

	MpvPlayer {
		id: player
		objectName: "player"
		anchors.fill: parent

		MouseArea {
			anchors.fill: parent
			acceptedButtons: Qt.RightButton

			onClicked: {
				if (mouse.button == Qt.RightButton) {
					playerMenu.popup()
				}
			}
		}
	}

	ContinueControls {
		visible: Video.continueData.show
	}

	TransportControls {
		visible: WindowController.controlsVisible
	}

	MouseArea {
		anchors.fill: parent
		enabled: false
		cursorShape: WindowController.controlsVisible ? Qt.ArrowCursor : Qt.BlankCursor
	}
}
