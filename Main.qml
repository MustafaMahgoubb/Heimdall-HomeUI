import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Window {
    id: root
    width: 1024
    height: 600
    visible: true
    color: "#121212"
    title: "Heimdall UI"

    // ==========================================
    // C++ & CommonAPI Bindings 
    // ==========================================
    property string versionNumber: typeof heimdall !== "undefined" ? heimdall.versionNumber : "loading..."
    property string updateStatus: typeof heimdall !== "undefined" ? heimdall.updateStatus : "checking..."

    signal checkForUpdatesClicked()
    signal goToMidgardClicked()
    signal goToAsgardClicked()
    // ==========================================

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: homePage
        
        pushEnter: Transition {
            PropertyAnimation { property: "opacity"; from: 0; to: 1; duration: 400 }
        }
        popEnter: Transition {
            PropertyAnimation { property: "opacity"; from: 0; to: 1; duration: 400 }
        }
    }

    // ----------------------------------------------------
    // HOME PAGE
    // ----------------------------------------------------
    Component {
        id: homePage
        Item {
            Image {
                anchors.fill: parent
                source: "qrc:/Images/resources/Heimdall_Bg.jpg"
                fillMode: Image.PreserveAspectCrop
                opacity: 0.5
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 40
                spacing: 50

                // Left Side - Navigation Buttons
                ColumnLayout {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                    spacing: 30

                    Button {
                        text: "Go to Midgard"
                        Layout.preferredHeight: 50
                        Layout.preferredWidth: 320
                        
                        background: Rectangle {
                            color: parent.pressed ? "#b8860b" : (parent.hovered ? "#daa520" : "#2a2a2a")
                            radius: 8
                            border.color: "#d4af37"
                            border.width: 1
                            Behavior on color { ColorAnimation { duration: 150 } }
                        }
                        contentItem: Text {
                            text: parent.text
                            font.pixelSize: 24
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            root.goToMidgardClicked()
                            stackView.push(midgardPage)
                        }
                    }

                    Button {
                        text: "Go to Asgard"
                        Layout.preferredHeight: 50
                        Layout.preferredWidth: 320
                        
                        background: Rectangle {
                            color: parent.pressed ? "#b8860b" : (parent.hovered ? "#daa520" : "#2a2a2a")
                            radius: 8
                            border.color: "#d4af37"
                            border.width: 1
                            Behavior on color { ColorAnimation { duration: 150 } }
                        }
                        contentItem: Text {
                            text: parent.text
                            font.pixelSize: 24
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            root.goToAsgardClicked()
                            stackView.push(asgardPage)
                        }
                    }

                    Button {
                        text: "Check For Updates"
                        Layout.preferredHeight: 50
                        Layout.preferredWidth: 320
                        
                        background: Rectangle {
                            color: parent.pressed ? "#b8860b" : (parent.hovered ? "#daa520" : "#2a2a2a")
                            radius: 8
                            border.color: "#d4af37"
                            border.width: 1
                            Behavior on color { ColorAnimation { duration: 150 } }
                        }
                        contentItem: Text {
                            text: parent.text
                            font.pixelSize: 24
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            if (typeof heimdall !== "undefined") heimdall.checkForUpdates()
                            root.checkForUpdatesClicked()
                        }
                    }
                }

                Item { Layout.fillWidth: true } // Spacer

                // Right Side - Status Panels
                ColumnLayout {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    spacing: 30

                    // Version Box
                    Rectangle {
                        Layout.preferredHeight: 60
                        Layout.preferredWidth: 320
                        color: "#1e1e1e"
                        radius: 8
                        border.color: "#444444"
                        opacity: 0.85

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 15
                            Text {
                                text: "Version:"
                                font.pixelSize: 22
                                color: "#aaaaaa"
                                Layout.alignment: Qt.AlignLeft
                            }
                            Item { Layout.fillWidth: true }
                            Text {
                                text: root.versionNumber // Bound to root property
                                font.pixelSize: 24
                                font.bold: true
                                color: "#d4af37" // Gold accent
                                Layout.alignment: Qt.AlignRight
                            }
                        }
                    }

                    // Update Status Box
                    Rectangle {
                        Layout.preferredHeight: 60
                        Layout.preferredWidth: 320
                        color: "#1e1e1e"
                        radius: 8
                        border.color: "#444444"
                        opacity: 0.85

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 15
                            Text {
                                text: "Updates:"
                                font.pixelSize: 22
                                color: "#aaaaaa"
                                Layout.alignment: Qt.AlignLeft
                            }
                            Item { Layout.fillWidth: true }
                            Text {
                                text: root.updateStatus // Bound to root property
                                font.pixelSize: 24
                                font.bold: true
                                color: root.updateStatus === "up-to-date" ? "#4caf50" : "#ff9800"
                                Layout.alignment: Qt.AlignRight
                            }
                        }
                    }
                }
            }
        }
    }

    // ----------------------------------------------------
    // MIDGARD PAGE
    // ----------------------------------------------------
    Component {
        id: midgardPage
        Item {
            Image {
                anchors.fill: parent
                source: "qrc:/Images/resources/Midgard.jpg"
                fillMode: Image.PreserveAspectCrop
                opacity: 0.8
            }

            Rectangle {
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 60
                color: "#aa000000"
                radius: 15
                width: title1.width + 60
                height: title1.height + 20

                Text {
                    id: title1
                    text: "Welcome to Midgard"
                    anchors.centerIn: parent
                    font.pixelSize: 42
                    font.bold: true
                    color: "white"
                }
            }

            Button {
                text: "Back to Home UI"
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 60
                width: 320
                height: 50
                
                background: Rectangle {
                    color: parent.pressed ? "#b8860b" : (parent.hovered ? "#daa520" : "#2a2a2a")
                    radius: 8
                    border.color: "#d4af37"
                    border.width: 1
                    Behavior on color { ColorAnimation { duration: 150 } }
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 24
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    stackView.pop()
                }
            }
        }
    }

    // ----------------------------------------------------
    // ASGARD PAGE
    // ----------------------------------------------------
    Component {
        id: asgardPage
        Item {
            Image {
                anchors.fill: parent
                source: "qrc:/Images/resources/asgard.jpg"
                fillMode: Image.PreserveAspectCrop
                opacity: 0.8
            }

            Rectangle {
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 60
                color: "#aa000000"
                radius: 15
                width: title2.width + 60
                height: title2.height + 20

                Text {
                    id: title2
                    text: "Welcome to Asgard"
                    anchors.centerIn: parent
                    font.pixelSize: 42
                    font.bold: true
                    color: "white"
                }
            }

            Button {
                text: "Back to Home UI"
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 60
                width: 320
                height: 50
                
                background: Rectangle {
                    color: parent.pressed ? "#b8860b" : (parent.hovered ? "#daa520" : "#2a2a2a")
                    radius: 8
                    border.color: "#d4af37"
                    border.width: 1
                    Behavior on color { ColorAnimation { duration: 150 } }
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 24
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    stackView.pop()
                }
            }
        }
    }
}
