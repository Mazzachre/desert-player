import QtQuick.Window 2.3
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
				icon {
					source: "icons/plus-circle.svg"
					width: 16
					height: 16
				}
				onClicked: PlaylistController.addPlaylist()
				focusPolicy: Qt.NoFocus
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
						icon {
							source: "icons/pencil.svg"
							width: 12
							height: 12
						}
						onClicked: {
							var buttonPosition = this.mapToItem(null, 0, 0)
							renamePlaylist.x = buttonPosition.x
							renamePlaylist.y = Math.min(buttonPosition.y, Window.height-110)
							renamePlaylist.id = id
							renamePlaylist.title = label
							renamePlaylist.open()
						}
						focusPolicy: Qt.NoFocus
					}

					Text {
						width: parent.width - 78
						text: label
						elide: Text.ElideRight
						padding: 3
						MouseArea {
							height: parent.height
							width: parent.width
							onClicked: Playlists.selectPlaylist(id)
						}
					}
					
					Text {
						width: 30
						text: fileCount
						padding: 3
						horizontalAlignment: Text.AlignRight
					}

					Button {
						width: 20
						height: 20
						enabled: id > 1
						icon {
							source: "icons/trash.svg"
							width: 12
							height: 12
						}
						onClicked: {
							var buttonPosition = this.mapToItem(null, 0, 0)
							deletePlaylist.x = buttonPosition.x
							deletePlaylist.y = Math.min(buttonPosition.y, Window.height-110)
							deletePlaylist.id = id
							deletePlaylist.open()
						}
						focusPolicy: Qt.NoFocus
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
				icon {
					source: "icons/play.svg"
					width: 16
					height: 16
				}
				onClicked: FileList.startPlaying()
				focusPolicy: Qt.NoFocus
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

			DropArea {
				anchors.fill: parent
				onEntered: if (drag.hasUrls) drag.accept(Qt.LinkAction)
				onDropped: if (drop.hasUrls) PlaylistController.addFiles(drop.urls)
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

	Popup {
		id: renamePlaylist
		parent: Overlay.overlay
		width: 250
		height: 100
		modal: true
		visible: false

		property int id: 0
		property string title: ""

		Rectangle {
			anchors.fill: parent
			
			TextField {
				anchors.top: parent.top
				anchors.horizontalCenter: parent.horizontalCenter
				selectByMouse: true
				width: parent.width - 20
				height: 30
				text: renamePlaylist.title
				onTextChanged: {
					renamePlaylist.title = text
				}
			}

			Row {
				anchors.bottom: parent.bottom
				width: parent.width
				height: 30
				spacing: 10

				Button {
					width: 108
					text: 'Cancel'
					onClicked: renamePlaylist.close()
				}
				Button {
					width: 108
					text: 'Save'
					onClicked: {
						PlaylistController.updateLabel(renamePlaylist.id, renamePlaylist.title)
						renamePlaylist.close()
					}
				}
			}
		}
	}

	Popup {
		id: deletePlaylist
		parent: Overlay.overlay
		width: 250
		height: 100
		modal: true
		visible: false

		property int id: 0

		Rectangle {
			anchors.fill: parent
			
			Text {
				anchors.top: parent.top
				anchors.horizontalCenter: parent.horizontalCenter
				width: parent.width - 20
				height: 30
				text: "Delete playlist?"
			}

			Row {
				anchors.bottom: parent.bottom
				width: parent.width
				height: 30
				spacing: 10

				Button {
					width: 108
					text: 'No'
					onClicked: deletePlaylist.close()
				}
				Button {
					width: 108
					text: 'Delete'
					onClicked: {
						PlaylistController.removePlaylist(deletePlaylist.id)
						deletePlaylist.close()
					}
				}
			}
		}
	}
}
