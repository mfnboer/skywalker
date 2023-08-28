import QtQuick
import QtQuick.Layouts
import skywalker

RowLayout {
    required property string postText
    required property list<imageview> postImages
    property var postExternal // externalview (var allows NULL)
    property var postRecord // recordview
    property var postRecordWithMedia // record_with_media_view
    property int maxTextLines: 1000

    id: postBody

    Text {
        id: bodyText
        width: parent.width
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        maximumLineCount: maxTextLines
        elide: Text.ElideRight
        text: postText
        bottomPadding: postImages.length > 0 || postExternal || postRecord ? 5 : 0
    }

    Component.onCompleted: {
        if (!postBody.visible)
            return

        if (postImages.length > 0) {
            let qmlFile = `ImagePreview${(postImages.length)}.qml`
            let component = Qt.createComponent(qmlFile)
            component.createObject(postBody.parent, {images: postImages})
        }

        if (postExternal) {
            let component = Qt.createComponent("ExternalView.qml")
            component.createObject(postBody.parent, {postExternal: postBody.postExternal})
        }

        if (postRecord) {
            let component = Qt.createComponent("RecordView.qml")
            component.createObject(postBody.parent, {record: postRecord})
        }

        if (postRecordWithMedia) {
            let component = Qt.createComponent("RecordWithMediaView.qml")
            component.createObject(postBody.parent, {record: postRecordWithMedia})
        }
    }
}
