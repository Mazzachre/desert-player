import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11
import desert 1.0

ListView {
	id: fileList
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

			Text {
				width: parent.width - 140
				text: title
				elide: Text.ElideRight
				padding: 3

				MouseArea {
					height: parent.height
					width: parent.width
					onClicked: FileList.selectFile(path)
					onDoubleClicked: {
						FileList.selectFile(path)
						FileList.startPlaying()
					}
				}
			}

			Text {
				width: 20
				text: wasStarted ? wasPlayed ? "\uE63B" : "\uE63A" : " "
				color: startedRecently ? "black" : "grey"
				font {
					family: icons.name
					pointSize: 10
				}
				padding: 3

				ToolTip {
					visible: wasStarted ? playedTT.containsMouse : false
					delay: 1000
					contentItem: Text {
						padding: 3
						text: playedList
						textFormat: Text.RichText
					}
				}

				MouseArea {
					id: playedTT
					anchors.fill: parent
					hoverEnabled: true
				}
			}

			Text {
				width: 20
				text: (hasSubtitle || internalSubtitle) ? "\uE645" : " "
				color: hasSubtitle ? "black" : "grey"
				font {
					family: icons.name
					pointSize: 10
				}
				padding: 3

				ToolTip {
					visible: (hasSubtitle || internalSubtitle) ? subtitleTT.containsMouse : false
					delay: 1000
					contentItem: Text {
						padding: 3
						text: subtitleTrack
						textFormat: Text.RichText
					}
				}

				MouseArea {
					id: subtitleTT
					anchors.fill: parent
					hoverEnabled: true
				}
			}

			Text {
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
				onClicked: PlaylistController.removeFile(id)
				focusPolicy: Qt.NoFocus
			}
		}
	}
}
