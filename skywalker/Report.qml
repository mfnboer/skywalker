import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var skywalker
    property basicprofile author
    property string postUri
    property string postCid
    property string postText
    property date postDateTime
    property generatorview feed
    property int reasonType: QEnums.REPORT_REASON_TYPE_NULL // QEnums.ReasonType
    property string details

    signal closed

    id: page
    width: parent.width
    height: parent.height
    topPadding: 10
    bottomPadding: 10

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        SvgButton {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            svg: svgOutline.cancel
            onClicked: page.closed()
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font.bold: true
            font.pointSize: guiSettings.scaledFont(10/8)
            color: guiSettings.headerTextColor
            text: postUri ? qsTr("Report post") : !author.isNull() ? qsTr("Report account") : qsTr("Report feed")
        }

        SkyButton {
            id: reportButton
            anchors.rightMargin: 10
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Send")
            visible: reasonType !== QEnums.REPORT_REASON_TYPE_NULL
            onClicked: sendReport()
        }
    }

    ListView {
        id: reasonList
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        x: 10
        width: parent.width - 20
        spacing: 0
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        model: reportUtils.reportReasons

        header: Rectangle {
            z: guiSettings.headerZLevel
            width: parent.width
            height: page.postUri ? quotePost.height : !page.author.isNull() ? authorRow.height : quoteFeed.height
            color: guiSettings.postHighLightColor
            border.width: 2
            border.color: guiSettings.borderColor

            RowLayout {
                id: authorRow
                width: parent.width
                spacing: 10
                visible: !postUri && !page.author.isNull()

                Rectangle {
                    width: 60
                    height: 60
                    color: "transparent"

                    Avatar {
                        x: 10
                        y: 10
                        width: 40
                        height: width
                        avatarUrl: author.avatarUrl
                    }
                }
                Column {
                    Layout.fillWidth: true

                    Text {
                        elide: Text.ElideRight
                        font.bold: true
                        color: guiSettings.textColor
                        text: author.name
                    }
                    Text {
                        elide: Text.ElideRight
                        font.pointSize: guiSettings.scaledFont(7/8)
                        color: guiSettings.handleColor
                        text: author.handle ? `@${author.handle}` : ""
                    }
                }
            }

            QuotePost {
                id: quotePost
                width: parent.width
                author: page.author
                postText: page.postText
                postDateTime: page.postDateTime
                visible: page.postUri
            }

            QuoteFeed {
                id: quoteFeed
                width: parent.width
                feed: page.feed
                visible: !page.feed.isNull();
            }
        }
        headerPositioning: ListView.OverlayHeader

        footer: SkyButton {
            z: guiSettings.headerZLevel
            anchors.horizontalCenter: parent.horizontalCenter
            text: details ? qsTr("Modify details") : qsTr("Add details")
            onClicked: page.editDetails()
        }
        footerPositioning: ListView.OverlayFooter

        delegate: RadioButton {
            required property reportreason modelData
            property alias reportReason: reasonEntry.modelData

            id: reasonEntry
            width: reasonList.width
            height: titleText.height + descriptionText.height
            leftPadding: 10
            rightPadding: 10

            contentItem: Column {
                id: buttonText
                anchors.top: parent.top
                anchors.left: reasonEntry.indicator.right
                anchors.leftMargin: 20
                width: parent.width - reasonEntry.indicator.width - 20

                Text {
                    id: titleText
                    topPadding: 5
                    width: parent.width - 20
                    wrapMode: Text.Wrap
                    font.bold: true
                    color: guiSettings.textColor
                    text: reasonEntry.reportReason.title
                }
                Text {
                    id: descriptionText
                    bottomPadding: 5
                    width: parent.width - 20
                    wrapMode: Text.Wrap
                    color: guiSettings.textColor
                    font.pointSize: guiSettings.scaledFont(7/8)
                    text: reasonEntry.reportReason.description
                }
            }

            onCheckedChanged: {
                if (checked)
                    page.reasonType = reasonEntry.reportReason.type
            }

            Component.onCompleted: {
                reasonEntry.indicator.x = 0
            }
        }
    }

    ReportUtils {
        id: reportUtils
        skywalker: page.skywalker

        onReportOk: {
            skywalker.showStatusMessage(qsTr("Report sent. Thank you for your report!"), QEnums.STATUS_LEVEL_INFO)
            page.closed()
        }

        onReportFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    GuiSettings {
        id: guiSettings
    }

    function editDetails() {
        let component = Qt.createComponent("ReportDetailsEditor.qml")
        let detailsPage = component.createObject(page, { text: page.details })
        detailsPage.onDetailsChanged.connect((text) => {
                page.details = text
                root.popStack()
        })
        root.pushStack(detailsPage)
    }

    function sendReport() {
        if (postUri) {
            reportUtils.reportPostOrFeed(postUri, postCid, reasonType, details)
        }
        else if (!author.isNull()) {
            reportUtils.reportAuthor(author.did, reasonType, details)
        }
        else {
            reportUtils.reportPostOrFeed(feed.uri, feed.cid, reasonType, details)
        }
    }
}
