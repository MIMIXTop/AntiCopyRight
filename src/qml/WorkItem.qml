import QtQuick
import QtQuick.Window

import MyModels

Rectangle {
    signal visibleTableOfStudensWorksSimilarity

    id: _worksItem
    height: 50
    width: parent.width
    radius: 10

    color: _actionArea.containsMouse ? "silver" : "white"
    border.width: 5
    border.color: _actionArea.containsMouse ? "blue" : "silver"

    Behavior on color {
        ColorAnimation {
            duration: 120
        }
    }

    Text {
        anchors.centerIn: parent
        text: name
    }

    MouseArea {
        id: _actionArea
        anchors.fill: parent
        hoverEnabled: true
    }
}
