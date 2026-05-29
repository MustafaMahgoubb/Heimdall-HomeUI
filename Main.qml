import QtQuick
import QtQuick.Layouts

Window {
    width: 1024
    height: 600
    visible: true
    flags: Qt.FramelessWindowHint
    color: "black"

    Image {
        id: background
        anchors.fill: parent
        source: "qrc:/Images/resources/Heimdall_Bg.jpg"

        fillMode: Image.PreserveAspectCrop
        opacity: 0.65
    }

    ColumnLayout
    {
        id: buttons
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 40
        spacing: 40

        Rectangle
        {
            id: button1
            Layout.preferredHeight: 40
            Layout.preferredWidth: 300
            color: "#5c5346"
            Text {
                id: button1txt
                text: qsTr("Button1")
                anchors.centerIn: parent
                font.pixelSize: 28
                color: "white"
            }

            MouseArea{
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton

                onClicked: (mouse)=> {
                               console.log("Mouse Clicked")
                           }

            }
        }

        Rectangle
        {
            id: button2
            Layout.preferredHeight: 40
            Layout.preferredWidth: 300
            color: "#5c5346"
            Text {
                id: button2txt
                anchors.centerIn: parent
                font.pixelSize: 28
                text: qsTr("Button2")
                color: "white"
            }

            MouseArea{
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton

                onClicked: (mouse)=> {
                               console.log("Mouse Clicked")
                           }

            }
        }

        Rectangle
        {
            id: button3
            Layout.preferredHeight: 40
            Layout.preferredWidth: 300
            color: "#5c5346"
            Text {
                id: button3txt
                anchors.centerIn: parent
                font.pixelSize: 28
                text: qsTr("Check For Updates")
                color: "white"
            }

            MouseArea{
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton

                onClicked: (mouse)=> {
                               console.log("Mouse Clicked")
                           }

            }
        }
    }

    ColumnLayout
    {
        id: status
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 40
        spacing: 40

        Rectangle
        {
            id: status1
            Layout.preferredHeight: 40
            Layout.preferredWidth: 300
            color: "#5c5346"
            RowLayout{
                Text {
                    id: versionState
                    text: qsTr("version: ")
                    Layout.alignment: Qt.AlignLeft
                    Layout.leftMargin: 10
                    font.pixelSize: 28
                    color: "white"
                }

                Text {
                    id: versionValue
                    Layout.alignment: Qt.AlignHCenter
                    Layout.rightMargin: 10
                    font.pixelSize: 28
                    color: "white"
                    text: qsTr("v-1.2.3")
                }
            }
        }


        Rectangle
        {
            id: status2
            Layout.preferredHeight: 40
            Layout.preferredWidth: 300
            color: "#5c5346"
            RowLayout{
                Text {
                    id: updateState
                    text: qsTr("updates: ")
                    Layout.alignment: Qt.AlignLeft
                    Layout.leftMargin: 10
                    font.pixelSize: 28
                    color: "white"
                }

                Text {
                    id: updateValue
                    Layout.alignment: Qt.AlignHCenter
                    Layout.rightMargin: 10
                    font.pixelSize: 28
                    color: "white"
                    text: qsTr("up-to-date")
                }
            }

        }
    }


}
