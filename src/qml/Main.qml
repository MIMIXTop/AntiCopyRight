import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: root
    width: Screen.desktopAvailableWidth
    height: Screen.desktopAvailableHeight
    visible: true
    title: "AntiCopyRight"

    RowLayout {
        id: _layout

        anchors.fill: root
        height: root.height
        width: 700

        SideBar {
            anchors.left: _layout.left
            height: _layout.height
            width: 300
        }
    }
}
