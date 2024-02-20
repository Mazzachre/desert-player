import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.3
import desert 1.0

Menu {
	id: subtitleMenu
	title: "Subtitles"
	width: 500

	MenuItem {
		text: "Add Subtitle File..."
		onTriggered: {
			selectSubtitle.folder = Subtitle.directory
			selectSubtitle.open()
		}
	}

	MenuItem {
		text: "Remove currently selected"
		enabled: Subtitle.external
		onTriggered: Subtitle.removeSubtitle()
	}

	MenuSeparator {
		visible: Subtitle.size > 0
		padding: 0
		topPadding: 12
		bottomPadding: 12
    }
    
	Instantiator {
		model: Subtitle
		delegate: MenuItem {
			contentItem: Row {
				Text {
					width: 20
					text: selected ? "\u2022" : ""
				}
				Text {
					text: label
				}
				Text {
					visible: external
					text: "(ext)"
				}
			}
			enabled: !selected
			onTriggered: player.changeSubtitleTrack(id);
		}
		onObjectAdded: subtitleMenu.insertItem(index+3, object)
		onObjectRemoved: subtitleMenu.removeItem(object)
	}
	
	FileDialog {
		id: selectSubtitle
		modality: Qt.WindowModal
		title: Subtitle.fileName
		folder: Subtitle.directory
		selectExisting: true
		selectMultiple: true
		selectFolder: false
		nameFilters: [ "Subtitle files (*.srt)", "All files (*)" ]
		selectedNameFilter: "Subtitle files (*.srt)"
		sidebarVisible: true
		onAccepted: {
			player.changeSubtitleTrack(fileUrl)
		}
	}
}
