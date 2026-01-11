import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Window
import QtQuick  // Для AnimatedImage

ApplicationWindow {
    height: 1080
    width: 1920
    title: qsTr("Hello")

    Rectangle {
        id: topBar
        height: 50
        width: parent.width
        color: "lightblue"

        Rectangle {
            id: sideBarButton
            property bool isOpen: sideBar.width > 0
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: 50
            height: 50
            color: "transparent"

            Image {
                id: buttonImage
                source: "qrc:/QML_SRC/resource/menu-64.svg"
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    if (sideBar.width != sideBar.widthMin) {
                        sideBar.width = sideBar.widthMin;
                    } else {
                        sideBar.width = sideBar.widthMax;
                    }
                }
            }
        }
    }

    Rectangle {
        id: sideBar
        anchors.top: topBar.bottom

        property int widthMin: 50
        property int widthMax: 300
        color: "red"
        height: parent.height - topBar.height
        width: widthMin

        Behavior on width {
            NumberAnimation {
                duration: 300
                easing.type: Easing.InOutQuad
            }
        }
    }
}
