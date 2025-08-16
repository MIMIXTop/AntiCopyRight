import QtQuick
import QtQuick.Controls

Rectangle {
    id: _backGroung

    property string name: ""

    Button {
        id: btn
        height: parent.height - 10
        width: parent.width - 10

        background: Rectangle {
            anchors.fill: parent
            color: btn.hovered ? "silver" : "white"

            radius: width / 2
            border {
                width: btn.down ? 7 : 4
                color: btn.hovered ? "blue" : "white"
            }
        }

        contentItem: Text {
            text: _backGroung.name
            font.family: "Verdana"
            font.pointSize: 15
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            anchors.left: parent.left
            anchors.leftMargin: 30
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
