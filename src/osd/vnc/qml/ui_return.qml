import QtQuick 1.1

Rectangle {
	Rectangle {
		id: titleRect
		anchors.fill: parent
		anchors.bottomMargin: parent.height - 20
		smooth: true
		border.color: "black"
		border.width: 1
		gradient: Gradient {
			GradientStop { position: 0.0; color: "lightblue" }
			GradientStop { position: 1.0; color: "darkblue" }
		}
		Text {
			id: titleText
			text: "Return to emulator"
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.verticalCenter: parent.verticalCenter
			font.pixelSize: 10
			font.bold: true
			color: "white"
			smooth: true
		}
	}
	Text {
		id: preliminaryText
		text: "Press enter to return to emulation!"
		anchors.fill: parent
		horizontalAlignment: Text.AlignHCenter
		verticalAlignment: Text.AlignVCenter
		color: "black"
		smooth: true
		font.pixelSize: 10
		font.bold: true
	}
	Keys.onPressed: {
		ui_overlay.exitRequested = ( event.key == Qt.Key_Enter || event.key == Qt.Key_Return );
	}
}
