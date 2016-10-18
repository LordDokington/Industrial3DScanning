import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2

import Renderer 1.0

ApplicationWindow {
    visible: true
    width: 640; height: 480

    title: qsTr("3D Scanning App")

    SceneRenderer {
        id: sceneRenderer

        anchors.fill: parent
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        property real lastX: 0
        property real lastY: 0

        onPressed: {
            lastX = mouse.x
            lastY = mouse.y
        }

        onPositionChanged: {
            var deltaX = mouse.x - lastX
            var deltaY = mouse.y - lastY
            lastX = mouse.x
            lastY = mouse.y

            console.log("update rotation:", deltaX, deltaY)

            var rotSpeed = 0.5;

            sceneRenderer.updateRotation(deltaX * rotSpeed, deltaY * rotSpeed)
        }
    }

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        height: 30

        color: "grey"

        RowLayout {
            anchors.fill: parent

            Button {
                text: "load model"

                Layout.fillHeight: true
                onClicked: selectGeometryFileDialog.open()
            }
        }
    }

    FileDialog {
        id: selectGeometryFileDialog
        title: "select file"
        folder: shortcuts.documents

        onAccepted: {
            var filePath = fileUrl.toString().replace( "file:///", "" )
            //console.log( "selected geometry file path:", filePath )

            sceneRenderer.geometryFilePath = filePath

            this.close()
        }

        onRejected: this.close()
    }
}
