import QtQuick 2.7
import QtQuick.Controls 1.4 as OldControls
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2

import Renderer 1.0

ApplicationWindow {
    visible: true
    width: 1024; height: 768

    title: qsTr("3D Scanning App")

    SceneRenderer {
        id: sceneRenderer

        anchors.fill: parent

        zDistance: 0.4 - zSlider.value

        usePerVertexColor: useVertexColorButton.checked
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

            sceneRenderer.rotate(lastX, lastY, mouse.x, mouse.y)

            lastX = mouse.x
            lastY = mouse.y
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

    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 50

        width: 120
        height: 90

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10

            OldControls.ExclusiveGroup { id: vertexColorGroup }

            Text {
                font.bold: true
                text: "vertex color"
            }

            OldControls.RadioButton {
                text: "uniform"
                exclusiveGroup: vertexColorGroup
            }
            OldControls.RadioButton {
                id: useVertexColorButton
                text: "KdTree"
                checked: true
                exclusiveGroup: vertexColorGroup
            }
        }
    }

    OldControls.Slider {
        id: zSlider

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom

            margins: 10
        }

        minimumValue: 0.0
        maximumValue: 0.3

        value: 0.1

        updateValueWhileDragging: true
    }

    FileDialog {
        id: selectGeometryFileDialog
        title: "select file"
        folder: shortcuts.documents

        onAccepted: {
            var filePath = fileUrl.toString().replace( "file:///", "" )
            console.log( "selected geometry file path:", filePath )

            sceneRenderer.geometryFilePath = filePath

            this.close()
        }

        onRejected: this.close()
    }
}
