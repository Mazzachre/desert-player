import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11
import desert 1.0

Rectangle {
	color: "grey"
	width: parent.width
	height: 40
	anchors.top: parent.top

	function format(seconds) {
		var hours = Math.floor(seconds / 3600);
		var minutes = Math.floor((seconds % 3600) / 60);
		var secondsLeft = seconds % 60;
		
		if (Video.duration > 3600) {
			return hours + ":" + String(minutes).padStart(2, '0') + ":" + String(secondsLeft).padStart(2, '0');
		} else {
			return minutes + ":" + String(secondsLeft).padStart(2, '0');
		}
	}

	Row {
		anchors.right: parent.right
		spacing: 10
		
		Text {
			text: "Last started: " + Video.continueData.started + " and played to: " + format(Video.continueData.position)
			anchors.verticalCenter: parent.verticalCenter
		}

		Button {
			text: "Continue"
			onClicked: Video.continuePlayback();
		}
	}
}
