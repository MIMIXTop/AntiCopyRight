import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import MyModels

Window {
    id: root
    width: Screen.desktopAvailableWidth
    height: Screen.desktopAvailableHeight
    visible: true
    title: "AntiCopyRight"

    WorksModel {
        id: works
    }

    WorksByCoursProxy {
        id: worksProxy
    }

    RowLayout {
        id: _layout

        height: root.height
        width: root.width

        SideBar {
            Layout.fillHeight: true
            spacing: 10
            width: 300
            onCourseSelected: {
                console.log(qsTr(courseId))
                works.updateWorks(courseId)
                worksProxy.selectedCourseId = courseId
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: works
            clip: true
            spacing: 10

            delegate: Rectangle {
                id: _worksItem
                width: parent.width
                anchors.margins: 10
                height: 50
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
        }
    }
}
