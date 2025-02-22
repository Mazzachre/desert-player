import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11
import QtQuick.Dialogs 1.3
import desert 1.0

Item {
	function openMenu(id, title, path, x, y) {
		filesMenu.fileId = id
		filesMenu.fileTitle = title
		filesMenu.filePath = path
		filesMenu.locX = x
		filesMenu.locY = y
		filesMenu.popup()
	}

	Menu {
		id: filesMenu

		property int fileId: 0
		property string fileTitle: ""
		property string filePath: ""
		property int locX: 0
		property int locY: 0

		MenuItem {
			text: "Rename"
			onTriggered: {
				renameFileTitle.x = filesMenu.locX
				renameFileTitle.y = filesMenu.locY
				renameFileTitle.fileTitle = filesMenu.fileTitle
				renameFileTitle.open()
			}
		}
		
		MenuItem {
			text: "Add subtitle"
			onTriggered: {
				var lastIndex = filesMenu.filePath.lastIndexOf("/")
				selectSubtitle.folder = "file://"+filesMenu.filePath.substring(0, lastIndex + 1)
				selectSubtitle.open()
			}
		}
	}

	FileDialog {
		id: selectExtras
		modality: Qt.WindowModal
		title: filesMenu.title
		folder: ""
		selectExisting: true
		selectMultiple: true
		selectFolder: false
		nameFilters: [
			"Video files (*.mp4 *.mkv *.avi *.mov *.wmv *.flv *.webm *.m4v *.mpg *.mpeg *.m2v *.ts *.m2ts *.vob *.3gp *.ogv *.rm *.rmvb *.asf *.divx)",
			"All files (*)"
		]
		selectedNameFilter: "Video files (*.mp4 *.mkv *.avi *.mov *.wmv *.flv *.webm *.m4v *.mpg *.mpeg *.m2v *.ts *.m2ts *.vob *.3gp *.ogv *.rm *.rmvb *.asf *.divx)"
		sidebarVisible: true
		onAccepted: {
			PlaylistController.addExtras(filesMenu.fileId, fileUrls);
		}
	}

	FileDialog {
		id: selectSubtitle
		modality: Qt.WindowModal
		title: Subtitle.fileName
		folder: Subtitle.directory
		selectExisting: true
		selectMultiple: false
		selectFolder: false
		nameFilters: [ "Subtitle files (*.srt *.sub)", "All files (*)" ]
		selectedNameFilter: "Subtitle files (*.srt *.sub)"
		sidebarVisible: true
		onAccepted: {
			FileList.updateSubtitle(filesMenu.fileId, fileUrl);
		}
	}

	Popup {
		id: renameFileTitle
		parent: Overlay.overlay
		width: 250
		height: 100
		modal: true
		visible: false

		property string fileTitle: ""

		Rectangle {
			anchors.fill: parent
			
			TextField {
				anchors.top: parent.top
				anchors.horizontalCenter: parent.horizontalCenter
				selectByMouse: true
				width: parent.width - 20
				height: 30
				text: renameFileTitle.fileTitle
				onTextChanged: {
					renameFileTitle.fileTitle = text
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
					onClicked: renameFileTitle.close()
				}
				Button {
					width: 108
					text: 'Save'
					onClicked: {
						FileList.updateFileTitle(filesMenu.fileId, renameFileTitle.fileTitle)
						renameFileTitle.close()
					}
				}
			}
		}
	}	
}
