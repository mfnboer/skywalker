import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required model

    signal hashtagClicked(string hashtag)

    id: searchList
    spacing: 0
    boundsBehavior: Flickable.StopAtBounds
    clip: true
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned

    Accessible.role: Accessible.List

    delegate: Rectangle {
        required property string modelData
        property alias hashtag: hashtagEntry.modelData

        id: hashtagEntry
        width: searchList.width
        height: hashtagText.height
        color: guiSettings.backgroundColor

        Accessible.role: Accessible.Button
        Accessible.name: hashtag
        Accessible.onPressAction: hashtagClicked(hashtagEntry.hashtag)

        Rectangle {
            width: parent.width
            height: 1
            color: guiSettings.separatorColor
        }

        SkyCleanedTextLine {
            id: hashtagText
            width: parent.width
            padding: 10
            elide: Text.ElideRight
            font.bold: true
            color: guiSettings.textColor
            plainText: `#${hashtagEntry.hashtag}`

            Accessible.ignored: true
        }

        MouseArea {
            z: -1
            anchors.fill: parent
            onClicked: hashtagClicked(hashtagEntry.hashtag)
        }
    }
}
