import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11
import desert 1.0

ListView {
	flickableDirection: Flickable.VerticalFlick
	boundsBehavior: Flickable.StopAtBounds
	ScrollBar.vertical: ScrollBar {
		policy: ScrollBar.AlwaysOn
		active: ScrollBar.AlwaysOn
	}
	clip: true
	model: FileList
	spacing: 5

	delegate: Rectangle {
		height: 24
		width: parent.width - 12
		color: selected ? "lightsteelblue" : index % 2 === 0 ? "ghostwhite" : "white"
		radius: 3

		Row {
			spacing: 5
			width: parent.width - 8
			height: parent.height - 4
			anchors.centerIn: parent

			Button {
				id: infoButton
				width: 20
				height: 20
				text: "\uE718"
				font {
					family: icons.name
					pointSize: 12
				}
				onClicked: FileList.showDetails(path);
			}

			Text {
				id: titleText
				width: parent.width - 140
				text: title
				elide: Text.ElideRight
				padding: 3
				MouseArea {
					height: parent.height
					width: parent.width
					onClicked: FileList.selectFile(path)
					onDoubleClicked: {
						FileList.selectFile(path);
						FileList.startPlaying();
					}
				}
			}

			Text {
				id: ccText
				width: 20
				text: hasSubtitle ? "\uF20A" : " "
				font {
					family: icons.name
					pointSize: 10
				}
				padding: 3
			}

			Text {
				id: durationText
				width: 60
				text: duration
				padding: 3
			}

			Button {
				width: 20
				height: 20
				text: "\uE605"
				font {
					family: icons.name
					pointSize: 12
				}
				onClicked: PlaylistController.removeFile(path)
			}
		}
	}
}
