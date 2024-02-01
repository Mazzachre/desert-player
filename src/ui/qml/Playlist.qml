import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11
import desert 1.0

Item {
	anchors.fill: parent

	Rectangle {
		id: playlists
		width: parent.width/3
		anchors {
			top: parent.top
			bottom: parent.bottom
			left: parent.left
		}
		
		Rectangle {
			id: playlistTools
			anchors {
				top: parent.top
				left: parent.left
				right: parent.right
			}
			height: 30
			gradient: Gradient {
				GradientStop { position: 0.0; color: "ghostwhite" }
				GradientStop { position: 0.33; color: "white" }
				GradientStop { position: 1.0; color: "lightsteelblue" }
			}
			Label {
				text: "Playlists"
				anchors {
					verticalCenter: parent.verticalCenter
					left: parent.left
					leftMargin: 4
				}
				font {
					pointSize: 14
					bold: true
				}
			}
			Button {
				width: 24
				height: 24
				anchors {
					verticalCenter: parent.verticalCenter
					right: parent.right
					rightMargin: 4
				}
				text: "\uE61A"
				font {
					family: icons.name
					pointSize: 14
					bold: true					
				}
				onClicked: PlaylistController.addPlaylist()
			}		
		}

		ListView {
			flickableDirection: Flickable.VerticalFlick
			boundsBehavior: Flickable.StopAtBounds
			ScrollBar.vertical: ScrollBar {
				policy: ScrollBar.AlwaysOn
				active: ScrollBar.AlwaysOn
			}
			anchors {
				top: playlistTools.bottom
				bottom: parent.bottom
				left: parent.left
				right: parent.right
				margins: 5
			}
			clip: true
			model: Playlists
			spacing: 5

			delegate: Rectangle {
				id: listView
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
						id: editButton
						width: 20
						height: 20
						enabled: id > 1
						text: "\ue61d"
						font {
							family: icons.name
							pointSize: 12
						}
						onClicked: {

							//TODO We need a way to edit the effin names!

						}
					}

					Text {
						width: parent.width - 48
						text: label
						elide: Text.ElideRight
						padding: 3
						MouseArea {
							height: parent.height
							width: parent.width
							onClicked: Playlists.selectPlaylist(id)
						}
					}

					Button {
						width: 20
						height: 20
						enabled: id > 1
						text: "\uE605"
						font {
							family: icons.name
							pointSize: 12
						}
						onClicked: PlaylistController.removePlaylist(id)
					}
				}
			}
		}
	}

	Rectangle {
		anchors {
			top: parent.top
			bottom: parent.bottom
			right: parent.right
			left: playlists.right
		}

		Rectangle {
			id: fileListTools
			height: 30
			anchors {
				top: parent.top
				left: parent.left
				right: parent.right
			}
			gradient: Gradient {
				GradientStop { position: 0.0; color: "ghostwhite" }
				GradientStop { position: 0.33; color: "white" }
				GradientStop { position: 1.0; color: "lightsteelblue" }
			}

			Button {
				id: playButton
				width: 34
				height: 24
				anchors {
					top: parent.top
					left: parent.left
					topMargin: 3
					leftMargin: 4
				}
				enabled: FileList.playable
				text: "\uE6AD"
				font {
					family: icons.name
					pointSize: 14
				}
				onClicked: FileList.startPlaying()
			}		

			Label {
				text: Playlists.label
				anchors {
					top: parent.top
					left: playButton.right
					leftMargin: 4
					topMargin: 3
				}
				font {
					pointSize: 14
					bold: true
				}
			}
		}


		Files {
			anchors {
				top: fileListTools.bottom
				bottom: parent.bottom
				left: parent.left
				right: parent.right
				margins: 5
			}
		}
	}

	Popup {
		parent: Overlay.overlay
		x: Math.round((parent.width - width) / 2)
		y: Math.round((parent.height - height) / 2)
		width: parent.width / 4
		height: parent.height / 4
		modal: true
		visible: PlaylistController.progress.show || false
		ProgressBar {
			anchors.fill: parent
			value: PlaylistController.progress.value || 0
			to: PlaylistController.progress.to || 0
			indeterminate: PlaylistController.progress.indeterminate || false
		}
	}
		
	DropArea {
		anchors.fill: parent
		onEntered: drag.accept (Qt.LinkAction)
		onDropped: PlaylistController.addFiles(drop.urls)
	}

}
