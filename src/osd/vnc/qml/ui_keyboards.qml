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
			text: "Keyboard mappings"
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
		text: "The keyboard mapper isn't working yet!"
		anchors.fill: parent
		horizontalAlignment: Text.AlignHCenter
		verticalAlignment: Text.AlignVCenter
		color: "red"
		smooth: true
		font.pixelSize: 10
		font.bold: true
	}
}
