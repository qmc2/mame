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
			text: "Client connections"
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.verticalCenter: parent.verticalCenter
			font.pixelSize: 10
			font.bold: true
			color: "white"
			smooth: true
		}
	}
	Rectangle {
		id: connListRect
		anchors.fill: parent
		anchors.margins: 5
		anchors.topMargin: 25
		color: "white"
		border.color: "black"
		border.width: 1
		Rectangle {
			height: 12
			width: connList.addressWidth
			border.color: "black"
			border.width: 1
			x: 1
			smooth: true
			gradient: Gradient {
				GradientStop { position: 0.0; color: "lightblue" }
				GradientStop { position: 1.0; color: "darkblue" }
			}
			Text {
				text: "IP address"
				font.pixelSize: 10
				width: connList.addressWidth
				smooth: true
				color: "white"
				anchors.fill: parent
				anchors.leftMargin: 2
			}
		}
		Rectangle {
			height: 12
			width: connList.hostnameWidth
			x: connList.addressWidth + 1
			border.color: "black"
			border.width: 1
			smooth: true
			gradient: Gradient {
				GradientStop { position: 0.0; color: "lightblue" }
				GradientStop { position: 1.0; color: "darkblue" }
			}
			Text {
				text: "Host name"
				font.pixelSize: 10
				width: connList.hostnameWidth
				smooth: true
				color: "white"
				anchors.fill: parent
				anchors.leftMargin: 2
			}
		}
		Rectangle {
			height: 12
			width: connList.portWidth
			x: connList.addressWidth + connList.hostnameWidth + 1
			border.color: "black"
			border.width: 1
			smooth: true
			gradient: Gradient {
				GradientStop { position: 0.0; color: "lightblue" }
				GradientStop { position: 1.0; color: "darkblue" }
			}
			Text {
				text: "Port"
				font.pixelSize: 10
				width: connList.hostnameWidth
				smooth: true
				color: "white"
				anchors.fill: parent
				anchors.leftMargin: 2
			}
		}
		ListView {
			id: connList
			property int addressWidth: connListRect.width/3
			property int hostnameWidth: connListRect.width - addressWidth - portWidth - 2
			property int portWidth: connListRect.width/5 - 15
			anchors.fill: connListRect
			anchors.topMargin: 12
			anchors.leftMargin: 2
			smooth: true
			model: connectionModel
			delegate: Rectangle {
				id: itemDelegate
				height: 16
				smooth: true
				Item {
					anchors.top: parent.top
					anchors.topMargin: 2
					smooth: true
					Row {
						Text {
							text: address
							color: "black"
							font.pixelSize: 10
							smooth: true
							width: connList.addressWidth + 1
							elide: Text.ElideRight
						}
						Text {
							text: hostname
							color: "black"
							font.pixelSize: 10
							smooth: true
							width: connList.hostnameWidth
							elide: Text.ElideRight
						}
						Text {
							text: port
							color: "black"
							font.pixelSize: 10
							smooth: true
							width: connList.portWidth
							elide: Text.ElideRight
						}
					}
				}
			}
		}
	}
}
