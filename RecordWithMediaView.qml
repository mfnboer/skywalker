import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Item {
    required property record_with_media_view record

    width: parent.width
    height: recordColumn.height + 10

    Column {
        id: recordColumn
        width: parent.width - 10
        anchors.centerIn: parent
        spacing: 5

        Component.onCompleted: {
            if (record.images.length > 0) {
                let qmlFile = `ImagePreview${(record.images.length)}.qml`
                let component = Qt.createComponent(qmlFile)
                component.createObject(recordColumn, {images: record.images})
            }

            if (record.external) {
                let component = Qt.createComponent("ExternalView.qml")
                component.createObject(recordColumn, {postExternal: record.external})
            }

            let component = Qt.createComponent("RecordView.qml")
            component.createObject(recordColumn, {record: record.record})
        }
    }
    Rectangle {
        anchors.fill: parent
        border.width: 1
        border.color: "lightgrey"
        color: "transparent"
        radius: 10
    }
}
