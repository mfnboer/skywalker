import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var skywalker
    property basicprofile author
    property string postUri
    property string postCid
    property string postText
    property date postDateTime
    property generatorview feed
    property listview list
    property string convoId
    property messageview message
    property int reasonType: QEnums.REPORT_REASON_TYPE_NULL // QEnums.ReasonType
    property string details
    readonly property int labelerAuthorListModelId: skywalker.createAuthorListModel(QEnums.AUTHOR_LIST_LABELERS, "")

    signal closed

    id: page
    width: parent.width
    height: parent.height
    topPadding: 10
    bottomPadding: 10
    Material.background: guiSettings.backgroundColor

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        SvgButton {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            svg: svgOutline.cancel
            accessibleName: qsTr("cancel")
            onClicked: page.closed()
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font.bold: true
            font.pointSize: guiSettings.scaledFont(10/8)
            color: guiSettings.headerTextColor
            text: getReportTitle()
        }

        SkyButton {
            id: reportButton
            anchors.rightMargin: 10
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Send")
            enabled: reasonType !== QEnums.REPORT_REASON_TYPE_NULL
            onClicked: sendReport()
        }
    }

    AccessibleText {
        id: labelerHeaderText
        x: 10
        width: parent.width - 20
        height: visible ? implicitHeight : 0
        font.pointSize: guiSettings.scaledFont(9/8)
        font.bold: true
        text: qsTr("Report to:")
        visible: labelerComboBox.visible
    }

    AuthorComboBox {
        id: labelerComboBox
        anchors.top: labelerHeaderText.bottom
        x: 10
        width: parent.width - 20
        height: visible ? implicitHeight : 0
        model: skywalker.getAuthorListModel(page.labelerAuthorListModelId)
        visible: count > 1 && message.isNull()
    }

    ListView {
        id: reasonList
        anchors.top: labelerComboBox.bottom
        anchors.bottom: parent.bottom
        x: 10
        width: parent.width - 20
        spacing: 0
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        model: reportUtils.getReportReasons(getReportTarget())

        header: Rectangle {
            z: guiSettings.headerZLevel
            width: parent.width
            height: calcHeight()
            radius: 3
            color: guiSettings.postHighLightColor
            border.width: 1
            border.color: guiSettings.borderColor

            function calcHeight() {
                if (quotePost.visible)
                    return quotePost.height

                if (quoteMessage.visible)
                    return quoteMessage.height

                if (authorRow.visible)
                    return authorRow.height

                if (quoteFeed.visible)
                    return quoteFeed.height

                if (quoteList.visible)
                    return quoteList.height
            }

            RowLayout {
                id: authorRow
                width: parent.width
                spacing: 10
                visible: !postUri && message.isNull() && !page.author.isNull()

                Rectangle {
                    width: 60
                    height: 60
                    color: "transparent"

                    Avatar {
                        x: 10
                        y: 10
                        width: 40
                        height: width
                        author: page.author
                    }
                }
                Column {
                    Layout.fillWidth: true

                    SkyCleanedText {
                        elide: Text.ElideRight
                        font.bold: true
                        color: guiSettings.textColor
                        plainText: author.name

                        Accessible.ignored: true
                    }
                    Text {
                        elide: Text.ElideRight
                        font.pointSize: guiSettings.scaledFont(7/8)
                        color: guiSettings.handleColor
                        text: author.handle ? `@${author.handle}` : ""

                        Accessible.ignored: true
                    }
                }
            }

            QuotePost {
                id: quotePost
                width: parent.width
                author: page.author
                postText: page.postText
                postDateTime: page.postDateTime
                ellipsisBackgroundColor: parent.color
                visible: page.postUri
            }

            QuotePost {
                id: quoteMessage
                width: parent.width
                author: page.author
                postText: page.message.formattedText
                postDateTime: message.sentAt
                ellipsisBackgroundColor: parent.color
                visible: !page.message.isNull()
            }

            QuoteFeed {
                id: quoteFeed
                width: parent.width
                feed: page.feed
                visible: !page.feed.isNull();
            }

            QuoteList {
                id: quoteList
                width: parent.width
                list: page.list
                visible: !page.list.isNull();
            }
        }
        headerPositioning: ListView.OverlayHeader

        footer: Rectangle {
            z: guiSettings.headerZLevel
            width: parent.width
            height: guiSettings.footerHeight
            color: "transparent"

            SkyButton {
                id: detailsButton
                anchors.horizontalCenter: parent.horizontalCenter
                text: details ? qsTr("Modify details") : qsTr("Add details")
                onClicked: page.editDetails()
            }
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
        let labelerDid = ""

        if (labelerComboBox.currentValue)
            labelerDid = labelerComboBox.currentValue.did

        if (postUri) {
            reportUtils.reportPostOrFeed(postUri, postCid, reasonType, details, labelerDid)
        }
        else if (!message.isNull()) {
            reportUtils.reportDirectMessage(message.senderDid, convoId, message.id, reasonType, details)
        }
        else if (!author.isNull()) {
            reportUtils.reportAuthor(author.did, reasonType, details, labelerDid)
        }
        else if (!feed.isNull()) {
            reportUtils.reportPostOrFeed(feed.uri, feed.cid, reasonType, details, labelerDid)
        }
        else if (!list.isNull()) {
            reportUtils.reportPostOrFeed(list.uri, list.cid, reasonType, details, labelerDid)
        }
    }

    function getReportTitle() {
        if (postUri)
            return "Report post"
        if (!message.isNull())
            return "Report message"
        if (!author.isNull())
            return "Report account"
        if (!feed.isNull())
            return "Report feed"
        if (!list.isNull())
            return "Report list"

        console.warn("UKNOWN REPORT")
        return "Report"
    }

    function getReportTarget() {
        if (postUri)
            return QEnums.REPORT_TARGET_POST
        if (!message.isNull())
            return QEnums.REPORT_DIRECT_MESSAGE
        if (!author.isNull())
            return QEnums.REPORT_TARGET_ACCOUNT
        if (!feed.isNull())
            return QEnums.REPORT_TARGET_FEED
        if (!list.isNull())
            return QEnums.REPORT_TARGET_LIST

        console.warn("UKNOWN REPORT TARGET")
        return QEnums.REPORT_TARGET_POST
    }

    Component.onDestruction: {
        skywalker.removeAuthorListModel(labelerAuthorListModelId)
    }

    Component.onCompleted: {
        skywalker.getAuthorList(labelerAuthorListModelId)
        console.debug("POST:", page.postText)
        console.debug("MSG:", page.message.formattedText)
    }
}
