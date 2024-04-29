import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")
    Button {
    text:"update"
    anchors.right:parent.right
    anchors.top: parent.top
    onClicked: {
      updatepage.visible = true
    }

    }
    UpdatePage {
        id: updatepage
        visible: false
    }
}
