import QtQuick
import QtQuick.Controls

import MyModels

ScrollView {
    ListView {
        CourseModel {
            id: courses
        }

        id: _SideBar
        model: courses
        delegate: CourseItem {
            name: nameCourse
            id: _backGroung
            anchors.leftMargin: 10
            height: 70
            width: parent.width
        }
    }
}
