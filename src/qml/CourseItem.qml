import QtQuick
import QtQuick.Controls

Rectangle {
    signal clicked
    property string name: ""
    id: btn

    color: _actionArea.containsMouse ? "silver" : "white"
    radius: width / 2

    border {
        width: 4
        color: _actionArea.containsMouse ? "blue" : "white"
    }
    Text {
        text: btn.name
        font.family: "Verdana"
        font.pointSize: 15
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        anchors.left: parent.left
        anchors.leftMargin: 30
        anchors.verticalCenter: parent.verticalCenter
    }

    Behavior on color {
        ColorAnimation {
            duration: 120
        }
    }

    Behavior on border.color {
        ColorAnimation {
            duration: 120
        }
    }

    MouseArea {
        id: _actionArea
        hoverEnabled: true
        anchors.fill: parent
        onClicked: btn.clicked()
    }
}
