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

    property bool justSmoothed: false
    property color uiColor: "#90DDDDDD"

    property bool redoSmoothingMode: false

    SceneRenderer {
        id: sceneRenderer

        anchors.fill: parent

        zDistance: 1.0 - zSlider.value

        usePerVertexColor: useVertexColorButton.checked
        pointSize: pointSizeSlider.value
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
        id: menuBar

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
        id: uiBar

        anchors.right: parent.right
        anchors.top: menuBar.bottom
        anchors.bottom: parent.bottom

        width: 220

        color: uiColor

        ColumnLayout {
            anchors.centerIn: parent

            spacing: 10

            Rectangle {
                Layout.alignment: Qt.AlignRight

                width: 200
                height: 90

                color: uiColor

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

            Rectangle {
                Layout.alignment: Qt.AlignRight

                width: 200
                height: 120

                color: uiColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10

                    Text {
                        font.bold: true
                        text: "smoothing"
                    }

                    RowLayout {
                        Text { text: "radius:" }

                        TextInput {
                            id: smoothRadiusInput
                            text: "0.005"
                            Layout.fillWidth: true

                            onTextChanged: { redoSmoothingMode = false }
                        }
                    }

                    RowLayout {
                        Button {
                            text: "smooth"

                            Layout.fillWidth: true

                            onClicked: {
                                if(redoSmoothingMode)
                                    sceneRenderer.undoSmooth()
                                else
                                {
                                    sceneRenderer.smoothMesh( parseFloat(smoothRadiusInput.text) )
                                }

                                justSmoothed = true
                                redoSmoothingMode = false
                            }
                        }

                        Button {
                            text: "undo"

                            enabled: justSmoothed

                            Layout.fillWidth: true

                            onClicked: {
                                sceneRenderer.undoSmooth()
                                justSmoothed = false
                                redoSmoothingMode = true
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.alignment: Qt.AlignRight

                width: 200
                height: 120

                color: uiColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10

                    Text {
                        font.bold: true
                        text: "normals"
                    }

                    RowLayout {
                        Text { text: "radius:" }

                        TextInput {
                            id: normalRadiusInput
                            text: "0.001"
                            Layout.fillWidth: true
                        }
                    }

                    RowLayout {
                        Button {
                            text: "estimate normals"

                            Layout.fillWidth: true

                            onClicked: {
                                sceneRenderer.estimateNormals( parseFloat(normalRadiusInput.text) )
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.alignment: Qt.AlignRight

                width: 200
                height: 120

                color: uiColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10

                    Text {
                        font.bold: true
                        text: "thinning"
                    }

                    RowLayout {
                        Text { text: "radius:" }

                        TextInput {
                            id: thinningRadiusInput
                            text: "0.001"
                            Layout.fillWidth: true
                        }
                    }

                    RowLayout {
                        Button {
                            text: "apply"

                            Layout.fillWidth: true

                            onClicked: {
                                sceneRenderer.thinning( parseFloat(thinningRadiusInput.text) )
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.alignment: Qt.AlignRight

                width: 200
                height: 120

                color: uiColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10

                    Text {
                        font.bold: true
                        text: "point size"
                    }

                    OldControls.Slider {
                        id: pointSizeSlider
                        Layout.fillWidth: true
                        value: 2.0

                        minimumValue: 0.5
                        maximumValue: 20.0
                    }
                }
            }
        }
    }

    OldControls.Slider {
        id: zSlider

        anchors {
            left: parent.left
            right: uiBar.left
            bottom: parent.bottom

            margins: 10
        }

        minimumValue: 0.0
        maximumValue: 1.0

        value: 0.7

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
            redoSmoothingMode = false

            this.close()
        }

        onRejected: this.close()
    }
}
