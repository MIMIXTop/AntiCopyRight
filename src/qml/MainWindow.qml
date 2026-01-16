import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Window
import Backend.GlobalState 1.0

ApplicationWindow {
    height: 1080
    width: 1920
    title: qsTr("Hello")
    GlobalState {
        id: globalState
    }

    Rectangle {
        id: topBar
        height: (parent.height / 100) * 8
        width: parent.width
        color: "lightblue"

        Rectangle {
            id: sideBarButton
            property bool isOpen: sideBar.width > 0
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: 64
            height: 64
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

        Rectangle {
            id: logoBar
            height: parent.height - 10
            width: parent.height - 10
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 5
            radius: height / 1.8
            border {
                color: "black"
                width: 1
            }

            MouseArea {
                anchors.fill: parent
                onClicked: authMenu.toggle()
            }

            Rectangle {
                id: userLogo
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                width: 50
                height: 50
                color: "transparent"
                radius: height / 1.8

                Image {
                    id: userLogoImage
                    source: globalState.logo !== "" ? globalState.logo : "qrc:/QML_SRC/resource/DefaultUserLogo.png"
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                }
            }
        }
    }

    Rectangle {
        id: authMenu
        width: 300
        height: 400
        color: "white"
        radius: 10
        transformOrigin: Item.TopRight
        clip: true
        opacity: 0
        border.color: "black"
        border.width: 1
        scale: 0
        visible: false
        anchors.topMargin: 10
        anchors.rightMargin: 10
        anchors.top: topBar.bottom
        anchors.right: parent.right

        Behavior on opacity {
            NumberAnimation {
                duration: 200
            }
        }

        Behavior on scale {
            NumberAnimation {
                duration: 300
                easing.type: Easing.OutBack
            }
        }

        function toggle() {
            if (visible) {
                opacity = 0;
                scale = 0;
                visible = false;
            } else {
                visible = true;
                scale = 1;
                opacity = 1;
            }
        }

        Text {
            id: authTitle
            width: parent.width
            anchors.centerIn: parent
            //anchors.verticalCenter: parent.verticalCenter
            text: "Здравствуйте,<br> для продолжения войдите в аккаунт"
            font.pixelSize: 20
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }

        Button {
            id: authBtn
            width: 120
            height: 65
            background: Rectangle {
                radius: parent.height / 1.8
                color: "lightblue"
            }
            text: "Войти"
            font.pixelSize: 18
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
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
