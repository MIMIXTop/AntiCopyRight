import QtQuick
import QtQuick.Window
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

            delegate: WorkItem {
                id: _worksItem
                anchors.margins: 10
            }
        }
    }
}
