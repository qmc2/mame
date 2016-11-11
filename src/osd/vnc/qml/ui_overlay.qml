import QtQuick 1.1

Rectangle {
	id: ui_overlay

	property string windowTitle: ""
	property bool exitRequested: false

	x: 0
	y: 0
	z: 0
	width: 400
	height: 400
	smooth: true
	color: "transparent"

	Rectangle {
		id: windowTitleRect
		anchors.top: parent.top
		width: parent.width
		height: 20
		border.color: "white"
		border.width: 2
		smooth: true
		gradient: Gradient {
			GradientStop { position: 0.0; color: "lightblue" }
			GradientStop { position: 1.0; color: "darkblue" }
		}
		Text {
			id: windowTitleText
			text: ui_overlay.windowTitle
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.verticalCenter: parent.verticalCenter
			font.pixelSize: 12
			font.bold: true
			color: "white"
			smooth: true
		}
	}

	Rectangle {
		id: canvasRect
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: windowTitleRect.bottom
		anchors.bottom: parent.bottom
		anchors.topMargin: 5
		width: parent.width
		border.color: "white"
		border.width: 2
		smooth: true
		gradient: Gradient {
			GradientStop { position: 0.0; color: "lightgray" }
			GradientStop { position: 1.0; color: "gray" }
		}
		Rectangle {
			id: menuRect
			anchors.fill: parent
			anchors.margins: 5
			anchors.rightMargin: 2 * parent.width/3
			color: "white"
			border.color: "black"
			border.width: 1
			ListView {
				id: menuView
				anchors.fill: parent
				anchors.margins: 5
				model: ListModel {
					ListElement {
						title: "Client connections"
					}
					ListElement {
						title: "Keyboard mappings"
					}
					ListElement {
						title: "Mouse mappings"
					}
					ListElement {
						title: "Chat console"
					}
					ListElement {
						title: "Return to emulator"
					}
				}
				delegate: Rectangle {
					id: menuDelegate
					height: 16
					smooth: true
					Item {
						anchors.top: parent.top
						anchors.topMargin: 2
						smooth: true
						Text {
							text: title
							color: "black"
							font.bold: true
							font.pixelSize: 10
							smooth: true
							width: menuView.width
							MouseArea {
								anchors.fill: parent
								onClicked: {
									menuView.currentIndex = index;
								}
								onDoubleClicked: {
									ui_overlay.exitRequested = ( index == 4 );
								}
							}
						}
					}
				}
				// we don't want the list-view to animate the item change
				highlightFollowsCurrentItem: false
				highlight: Rectangle {
					width: menuRect.width - 6
					height: 16
					x: -2
					y: menuView.currentItem.y
					color: "lightgrey"
					radius: 4
					border.color: "black"
					border.width: 1
				}
				focus: true
				keyNavigationWraps: true
				Keys.onPressed: {
					switch ( event.key ) {
						case Qt.Key_Home:
							currentIndex = 0;
							positionViewAtBeginning();
							event.accepted = true;
							break;
						case Qt.Key_End:
							currentIndex = count - 1;
							positionViewAtEnd();
							event.accepted = true;
							break;
					}
				}
			}
		}

		ConnectionView {
			id: connectionView
			visible: menuView.currentIndex == 0
			anchors.fill: parent
			anchors.margins: 5
			anchors.leftMargin: parent.width/3 + 5
			border.color: "black"
			border.width: 1
			color: "transparent"
		}

		KeyboardMapper {
			id: keyboardMapper
			visible: menuView.currentIndex == 1
			anchors.fill: parent
			anchors.margins: 5
			anchors.leftMargin: parent.width/3 + 5
			border.color: "black"
			border.width: 1
			color: "transparent"
		}

		MouseMapper {
			id: mouseMapper
			visible: menuView.currentIndex == 2
			anchors.fill: parent
			anchors.margins: 5
			anchors.leftMargin: parent.width/3 + 5
			border.color: "black"
			border.width: 1
			color: "transparent"
		}

		ChatConsole {
			id: chatConsole
			visible: menuView.currentIndex == 3
			anchors.fill: parent
			anchors.margins: 5
			anchors.leftMargin: parent.width/3 + 5
			border.color: "black"
			border.width: 1
			color: "transparent"
		}

		EmulatorReturn {
			id: emulatorReturn
			visible: menuView.currentIndex == 4
			anchors.fill: parent
			anchors.margins: 5
			anchors.leftMargin: parent.width/3 + 5
			border.color: "black"
			border.width: 1
			color: "transparent"
		}
	}

	Keys.forwardTo: [menuView, emulatorReturn]
}
