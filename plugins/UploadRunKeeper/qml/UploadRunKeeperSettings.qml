import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page
    SilicaFlickable {
        anchors.fill: parent
        contentHeight: childrenRect.height
        Column {
            id: column
            width: page.width
            spacing: Theme.paddingLarge
            PageHeader {
                title: qsTr("RunKeeper plugin")
            }
            Label {
                anchors.horizontalCenter: column.horizontalCenter
                text: "runkeeper"
            }
        }
    }
}
