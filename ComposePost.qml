import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import skywalker

Page {
    property int maxPostLength: 300
    property int maxImages: 4
    property list<string> images
    signal closed

    id: page
    topPadding: 10
    bottomPadding: 10

    header: Rectangle {
        width: parent.width
        height: root.headerHeight
        z: 10
        color: "black"

        Button {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            Material.background: "blue"
            contentItem: Text {
                color: "white"
                text: qsTr("Cancel")
            }

            onClicked: page.closed()
        }

        Button {
            id: postButton
            anchors.rightMargin: 10
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            Material.background: "blue"

            contentItem: Text {
                color: "white"
                text: qsTr("Post", "verb on post button")
            }

            enabled: postText.textLength() <= maxPostLength && (postText.textLength() > 0 || page.images.length > 0)
            onClicked: {
                postButton.enabled = false
                skywalker.post(postText.text, images);
            }
        }
    }

    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: postText.width
        contentHeight: postText.height + imageScroller.height
        flickableDirection: Flickable.VerticalFlick

        TextArea {
            id: postText
            width: page.width
            placeholderText: textLength() === 0 ? "Say something nice" : ""
            placeholderTextColor: "grey"
            wrapMode: TextEdit.Wrap
            font.pointSize: root.scaledFont(9/8)
            clip: true
            focus: true
            background: Rectangle { border.color: "transparent" }

            onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)

            function textLength() {
                return length + preeditText.length
            }
        }

        Rectangle {
            id: textFooter
            anchors.top: postText.bottom
            anchors.topMargin: 30
            width: page.width
            height: root.footerHeight
            z: 10
            color: "transparent"

            ProgressBar {
                id: textLengthBar
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                from: 0
                to: page.maxPostLength
                value: postText.textLength()

                contentItem: Rectangle {
                    width: textLengthBar.visualPosition * parent.width
                    height: parent.height
                    color: postText.textLength() <= maxPostLength ? "blue" : "red"
                }
            }

            SvgImage {
                x: 10
                y: height + 5
                width: 34
                height: 34
                id: homeButton
                color: page.images.length < maxImages ? "blue" : "lightgrey"
                svg: svgOutline.addImage

                MouseArea {
                    y: -parent.y
                    width: parent.width
                    height: parent.height
                    visible: page.images.length < maxImages

                    onClicked: {
                        if (Qt.platform.os === "android")
                            skywalker.pickPhoto()
                        else
                            fileDialog.open()
                    }
                }
            }

            Text {
                anchors.rightMargin: 10
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                color: postText.textLength() <= maxPostLength ? Material.foreground : "red"
                text: maxPostLength - postText.textLength()
            }
        }

        ScrollView {
            property int imgWidth: 240

            id: imageScroller
            height: 180
            width: page.width
            horizontalPadding: 10
            anchors.top: textFooter.bottom
            contentWidth: imageRow.width
            contentHeight: height

            Row {
                id: imageRow
                width: page.images.length * imageScroller.imgWidth + (page.images.length - 1) * spacing
                spacing: 10

                Repeater {
                    model: page.images

                    Image {
                        required property string modelData
                        required property int index

                        width: imageScroller.imgWidth
                        height: imageScroller.height
                        fillMode: Image.PreserveAspectCrop
                        autoTransform: true
                        source: "file://" + modelData

                        onStatusChanged: {
                            if (status === Image.Error)
                            {
                                statusPopup.show(qsTr("Cannot load image"), QEnums.STATUS_LEVEL_ERROR);
                                page.images.splice(index, 1)
                            }
                        }

                        RoundButton {
                            Material.background: "black"
                            contentItem: Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                color: "white"
                                font.pointSize: root.scaledFont(7/8)
                                font.bold: true
                                text: qsTr("+ALT", "add alternative text button")
                            }
                        }

                        SvgButton {
                            x: parent.width - width
                            height: width
                            iconColor: "white"
                            Material.background: "black"
                            opacity: 1
                            svg: svgOutline.close
                            onClicked: page.images.splice(index, 1)
                        }
                    }
                }
            }
        }

        function ensureVisible(r) {
            if (contentY >= r.y)
                contentY = r.y;
            else if (contentY + height <= r.y + r.height)
                contentY = r.y + r.height - height;
        }
    }

    StatusPopup {
        id: statusPopup
    }

    FileDialog {
        id: fileDialog
        currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
        nameFilters: ["Image files (*.jpg *.jpeg *.png)"]
        onAccepted: {
            let fileName = selectedFile.toString()
            if (fileName.startsWith("file://"))
                fileName = fileName.substr(7)

            photoPicked(fileName)
        }
    }

    function postFailed(error) {
        console.debug(error)
        statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        postButton.enabled = true
    }

    function postProgress(msg) {
        console.debug(msg)
        statusPopup.show(msg, QEnums.STATUS_LEVEL_INFO)
    }

    function photoPicked(fileName) {
        console.debug("IMAGE:", fileName)
        page.images.push(fileName)
        let scrollBar = imageScroller.ScrollBar.horizontal
        scrollBar.position = 1.0 - scrollBar.size
    }

    function postDone() {
        page.closed()
    }

    Component.onDestruction: {
        skywalker.photoPicked.disconnect(photoPicked)
        skywalker.postOk.disconnect(postDone)
        skywalker.postFailed.disconnect(postFailed)
        skywalker.postProgress.disconnect(postProgress)
    }

    Component.onCompleted: {
        skywalker.photoPicked.connect(photoPicked)
        skywalker.postOk.connect(postDone)
        skywalker.postFailed.connect(postFailed)
        skywalker.postProgress.connect(postProgress)
    }
}
