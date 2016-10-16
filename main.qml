import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import Renderer 1.0

ApplicationWindow {
    visible: true
    width: 800; height: 600

    title: qsTr("3D Scanning App")

    SceneRenderer {
        anchors.fill: parent
    }

    RowLayout {
        anchors.top: parent.top
        anchors.left: parent.left

        anchors.margins: 5

        Button {
            text: "load model"
        }
    }
}
