import QtQuick 2.0

ListView {
    width: 100; height: 100
    anchors.fill: parent

    model: myModel
    delegate: Rectangle {
        height: 25
        width: 100
        Text { text: title }
    }
}

