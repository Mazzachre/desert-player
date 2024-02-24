import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11
import desert 1.0

Rectangle {
	id: transportControls
	color: "grey"
	opacity: 0.7
	width: parent.width
	height: 100
	anchors.bottom: parent.bottom

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

	Column {
		anchors.fill: parent

		Slider {
			id: position
			from: 0
			to: Video.duration
			value: Video.position
			height: parent.height / 2
			anchors {
				left: parent.left
				right: parent.right
			}

			ToolTip.visible: hovered
			ToolTip.text: format(Math.round((positionMouseArea.mouseX/position.width)*Video.duration))

			MouseArea {
				id: positionMouseArea
				anchors.fill: parent
				hoverEnabled: true
				onClicked: Video.positionChange(Math.round((positionMouseArea.mouseX/position.width)*Video.duration));
			}

			background: Rectangle {
				id: positionSlider
				x: position.leftPadding
				y: position.topPadding + position.availableHeight / 2 - height / 2
				implicitWidth: 200
				implicitHeight: 4
				width: position.availableWidth
				height: implicitHeight
				radius: 2
				color: "#f0f0f0"

				Rectangle {
					width: position.visualPosition * parent.width
					height: parent.height
					color: "#000"
					radius: 2
				}
			}

			handle: Rectangle {
				width: 8
				height: 20
				x: position.leftPadding + position.visualPosition * (position.availableWidth - width)
				y: position.topPadding + position.availableHeight / 2 - height / 2
				color: "lightgrey"
				radius: 5

				MouseArea {
					anchors.fill: parent
					drag.target: parent
					drag.axis: Drag.XAxis
					drag.minimumX: positionSlider.x
					drag.maximumX: positionSlider.width

					onReleased: {
						Video.positionChange(((parent.x-positionSlider.x) / position.width)*Video.duration)
					}
				}
			}
		}

		Item {
			height: parent.height / 2
			anchors {
				left: parent.left
				right: parent.right
				leftMargin: 5
			}

			Row {
				spacing: 5
				height: parent.height
				anchors {
					left: parent.left
				}

				TextMetrics {
					id: positionText
					font.family: "Ariel"
					text: "0:00:00 / 0:00:00"
				}

				Rectangle {
					color: "lightgrey"
					width: positionText.width + 20
					height: transportControls.height / 3
					anchors.verticalCenter: parent.verticalCenter

					Text {
						anchors.centerIn: parent
						font.family: "Ariel"
						text: format(Video.position) + " / " + format(Video.duration)
					}
				}

				Button {
					enabled: FileList.hasPrev
					anchors.verticalCenter: parent.verticalCenter
					width: transportControls.height / 3
					height: transportControls.height / 3
					font.family: icons.name
					font.pixelSize: transportControls.height / 4
					text: "\uE723"
					onClicked: FileList.playPrev()
					focusPolicy: Qt.NoFocus
				}

				Button {
					anchors.verticalCenter: parent.verticalCenter
					width: transportControls.height / 3
					height: transportControls.height / 3
					font.family: icons.name
					font.pixelSize: transportControls.height / 4
					text: Video.paused ? "\uE6AD" : "\uE6AE"
					onClicked: player.togglePause()
					focusPolicy: Qt.NoFocus
				}

				Button {
					anchors.verticalCenter: parent.verticalCenter
					width: transportControls.height / 3
					height: transportControls.height / 3
					font.family: icons.name
					font.pixelSize: transportControls.height / 4
					text: "\uE6AB"
					onClicked: player.stopMovie()
					focusPolicy: Qt.NoFocus
				}

				Button {
					enabled: FileList.hasNext
					anchors.verticalCenter: parent.verticalCenter
					width: transportControls.height / 3
					height: transportControls.height / 3
					font.family: icons.name
					font.pixelSize: transportControls.height / 4
					text: "\uE722"
					onClicked: FileList.playNext()
					focusPolicy: Qt.NoFocus
				}
				
				Button {
					anchors.verticalCenter: parent.verticalCenter
					width: transportControls.height / 3
					height: transportControls.height / 3
					font.family: icons.name
					font.pixelSize: transportControls.height / 4
					text: WindowController.fullScreen ? "\uE6C9" : "\uE659"
					onClicked: WindowController.toggleFullScreen()
					focusPolicy: Qt.NoFocus
				}
			}

			Slider {
				height: parent.height
				anchors {
					right: parent.right
				}

				id: volume
				from: 0
				to: 100
				width: 150
				enabled: !Audio.muted
				opacity: Audio.muted ? 0.3 : 1
				value: Audio.volume
				onMoved: Audio.changeVolume(volume.value)
				
				background: Rectangle {
					x: volume.leftPadding
					y: volume.topPadding + volume.availableHeight / 2 - height / 2
					implicitWidth: 200
					implicitHeight: 4
					width: volume.availableWidth
					height: implicitHeight
					radius: 2
					color: "#000"

					Rectangle {
						width: volume.visualPosition * parent.width
						height: parent.height
						color: "#f0f0f0"
						radius: 2
					}
				}

				handle: Rectangle {
					x: volume.leftPadding + volume.visualPosition * (volume.availableWidth - width)
					y: volume.topPadding + volume.availableHeight / 2 - height / 2
					implicitWidth: 16
					implicitHeight: 26
					radius: 13
					color: volume.pressed ? "#f0f0f0" : "#f6f6f6"
					border.color: "#bdbebf"
				}
			}
		}
	}
}
