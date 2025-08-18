import QtQuick
import QtQuick.Controls

import MyModels

ListView {
    signal courseSelected(string courseId)
    CourseModel {
        id: courses
    }

    id: _SideBar
    model: courses
    delegate: CourseItem {
        name: nameCourse
        height: 50
        width: parent.width

        onClicked: {
            _SideBar.courseSelected(courseId)
            console.log(courseId)
        }
    }
}
