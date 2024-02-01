import QtQuick 2.12
import QtQuick.Controls 2.12
import desert 1.0

Menu {
	id: videoMenu
    title: "Video"
    width: 350

	Instantiator {
		model: Video
		delegate: MenuItem {
			contentItem: Row {
				Text {
					width: 20
					text: selected ? "\u2022" : ""
				}
				Text {
					text: label + " - " + codec
				}
			}
			enabled: !selected
			onTriggered: player.changeVideoTrack(id);
		}
		onObjectAdded: videoMenu.insertItem(index, object)
		onObjectRemoved: videoMenu.removeItem(object)
	}
}
