import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property basicprofile author
    property string postUri
    property string postCid
    property string postText
    property date postDateTime
    property generatorview feed
    property listview list
    property starterpackview starterPack
    property string convoId
    property messageview message
    property int categoryType: QEnums.REPORT_CATEGORY_TYPE_NULL // QEnums.CategoryType
    property int reasonType: QEnums.REPORT_REASON_TYPE_NULL // QEnums.ReasonType
    readonly property string details: detailsText.text
    readonly property int labelerAuthorListModelId: skywalker.createAuthorListModel(QEnums.AUTHOR_LIST_LABELERS, "")

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

        SvgPlainButton {
            id: cancelButton
            y: (parent.height - height) / 2
            anchors.left: parent.left
            svg: SvgOutline.cancel
            accessibleName: qsTr("cancel")
            onClicked: page.closed()
        }

        AccessibleText {
            y: (parent.height - height) / 2
            anchors.left: cancelButton.right
            anchors.right: reportButton.left
            font.bold: true
            font.pointSize: guiSettings.scaledFont(10/8)
            color: guiSettings.headerTextColor
            text: getReportTitle()
        }

        SkyButton {
            id: reportButton
            y: (parent.height - height) / 2
            anchors.right: avatar.left
            text: qsTr("Send")
            enabled: reasonType !== QEnums.REPORT_REASON_TYPE_NULL
            onClicked: sendReport()
        }

        Loader {
            id: avatar
            y: (parent.height - height) / 2
            anchors.rightMargin: 10
            anchors.right: parent.right
            height: parent.height - 10
            width: active ? height : 0
            active: !root.isActiveUser(userDid)

            sourceComponent: CurrentUserAvatar {
                userDid: page.userDid
            }
        }
    }

    footer: Rectangle {
        id: pageFooter
        width: page.width
        height: guiSettings.footerHeight + (keyboardHandler.keyboardVisible ? keyboardHandler.keyboardHeight : 0)
        z: guiSettings.footerZLevel
        color: guiSettings.footerColor
        visible: detailsText.activeFocus

        TextLengthBar {
            textField: detailsText
        }

        TextLengthCounter {
            y: 10
            anchors.rightMargin: 10
            anchors.right: parent.right
            textField: detailsText
        }
    }

    Rectangle {
        id: quotedText
        x: 10
        width: parent.width - 20
        height: calcHeight()
        radius: 3
        color: guiSettings.postHighLightColor
        border.width: 1
        border.color: guiSettings.borderHighLightColor

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

            if (quoteStarterPack.visible)
                return quoteStarterPack.height
        }

        RowLayout {
            id: authorRow
            width: parent.width
            spacing: 10
            visible: !postUri && message.isNull() && !page.author.isNull()

            Rectangle {
                Layout.preferredWidth: 60
                Layout.preferredHeight: 60
                color: "transparent"

                Avatar {
                    x: 10
                    y: 10
                    width: 40
                    userDid: page.userDid
                    author: page.author
                }
            }
            Column {
                Layout.fillWidth: true

                AuthorNameAndStatus {
                    width: parent.width
                    userDid: page.userDid
                    author: page.author
                }
                AccessibleText {
                    width: parent.width
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
            userDid: page.userDid
            author: page.author
            postText: page.postText
            postDateTime: page.postDateTime
            postBackgroundColor: parent.color.toString()
            visible: page.postUri
        }

        QuotePost {
            id: quoteMessage
            width: parent.width
            userDid: page.userDid
            author: page.author
            postText: page.message.formattedText
            postDateTime: message.sentAt
            postBackgroundColor: parent.color.toString()
            visible: !page.message.isNull()
        }

        QuoteFeed {
            id: quoteFeed
            width: parent.width
            userDid: page.userDid
            feed: page.feed
            visible: !page.feed.isNull();
        }

        QuoteList {
            id: quoteList
            width: parent.width
            userDid: page.userDid
            list: page.list
            visible: !page.list.isNull();
        }

        QuoteStarterPack {
            id: quoteStarterPack
            width: parent.width
            userDid: page.userDid
            starterPack: page.starterPack
            visible: !page.starterPack.isNull()
        }
    }

    Flickable {
        id: flick
        anchors.top: quotedText.bottom
        anchors.topMargin: 10
        anchors.bottom: parent.bottom
        clip: true
        width: parent.width
        contentHeight: contentItem.childrenRect.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: SkyScrollBarVertical {}

        AccessibleText {
            id: labelerHeaderText
            x: 10
            width: parent.width - 20
            height: visible ? implicitHeight : 0
            font.pointSize: guiSettings.scaledFont(9/8)
            font.bold: true
            elide: Text.ElideRight
            text: qsTr("1. Report to:")
        }

        AuthorComboBox {
            id: labelerComboBox
            anchors.top: labelerHeaderText.bottom
            anchors.topMargin: 10
            x: 10
            width: parent.width - 20
            height: visible ? implicitHeight : 0
            model: skywalker.getAuthorListModel(page.labelerAuthorListModelId)
            enabled: count > 1 && message.isNull()
        }

        AccessibleText {
            id: categoryHeader
            anchors.top: labelerComboBox.bottom
            topPadding: 10
            bottomPadding: 10
            x: 10
            width: parent.width - 20
            font.pointSize: guiSettings.scaledFont(9/8)
            font.bold: true
            elide: Text.ElideRight
            text: qsTr("2. Why should this post be reviewed?")
        }

        Rectangle {
            id: categorySection
            anchors.top: categoryHeader.bottom
            x: 10
            width: parent.width - 20
            height: categoryList.height + 20
            radius: guiSettings.radius
            color: guiSettings.textInputBackgroundColor

            ListView {
                id: categoryList
                x: 10
                y: 10
                width: parent.width - 20
                height: contentHeight
                spacing: 0
                boundsBehavior: Flickable.StopAtBounds
                clip: true
                model: reportUtils.getReportCategories()

                delegate: SkyRoundRadioButton {
                    required property reportcategory modelData
                    property alias categoryReason: categoryEntry.modelData

                    id: categoryEntry
                    width: categoryList.width
                    height: titleText.height + descriptionText.height
                    leftPadding: 10
                    rightPadding: 10

                    contentItem: Column {
                        id: buttonText
                        anchors.top: parent.top
                        anchors.left: categoryEntry.indicator.right
                        anchors.leftMargin: 20
                        width: parent.width - categoryEntry.indicator.width - 20

                        AccessibleText {
                            id: titleText
                            topPadding: 5
                            width: parent.width - 20
                            wrapMode: Text.Wrap
                            font.bold: true
                            text: categoryEntry.categoryReason.title
                        }
                        AccessibleText {
                            id: descriptionText
                            bottomPadding: 5
                            width: parent.width - 20
                            wrapMode: Text.Wrap
                            font.pointSize: guiSettings.scaledFont(7/8)
                            text: categoryEntry.categoryReason.description
                        }
                    }

                    onCheckedChanged: {
                        if (checked)
                        {
                            page.reasonType = QEnums.REPORT_REASON_TYPE_NULL
                            page.categoryType = categoryEntry.categoryReason.type
                        }
                    }

                    Component.onCompleted: {
                        categoryEntry.indicator.x = 0
                    }
                }
            }
        }

        AccessibleText {
            id: reasonHeader
            anchors.top: categorySection.bottom
            topPadding: 10
            bottomPadding: 10
            x: 10
            width: parent.width - 20
            font.pointSize: guiSettings.scaledFont(9/8)
            font.bold: true
            elide: Text.ElideRight
            text: qsTr("3. Select a reason")
        }

        Rectangle {
            id: reasonSection
            anchors.top: reasonHeader.bottom
            x: 10
            width: parent.width - 20
            height: reasonList.height + 20
            radius: guiSettings.radius
            color: guiSettings.textInputBackgroundColor
            visible: page.categoryType !== QEnums.REPORT_CATEGORY_TYPE_NULL

            ListView {
                id: reasonList
                x: 10
                y: 10
                width: parent.width - 20
                height: contentHeight
                spacing: 5
                boundsBehavior: Flickable.StopAtBounds
                clip: true
                model: reportUtils.getReportReasons(page.categoryType)

                onModelChanged: {
                    if (model.length === 1)
                        page.reasonType = model[0].type
                }

                delegate: SkyRoundRadioButton {
                    required property reportreason modelData
                    property alias reportReason: reasonEntry.modelData

                    id: reasonEntry
                    width: reasonList.width
                    height: reasonTitleText.height
                    leftPadding: 10
                    rightPadding: 10

                    contentItem: Column {
                        anchors.top: parent.top
                        anchors.left: reasonEntry.indicator.right
                        anchors.leftMargin: 20
                        width: parent.width - reasonEntry.indicator.width - 20

                        AccessibleText {
                            id: reasonTitleText
                            topPadding: 5
                            bottomPadding: 5
                            width: parent.width - 20
                            wrapMode: Text.Wrap
                            font.bold: true
                            text: reasonEntry.reportReason.title
                        }
                    }

                    checked: page.reasonType === reasonEntry.reportReason.type

                    onCheckedChanged: {
                        if (checked)
                            page.reasonType = reasonEntry.reportReason.type
                    }

                    Component.onCompleted: {
                        reasonEntry.indicator.x = 0
                    }
                }
            }
        }

        AccessibleText {
            id: detailsHeader
            anchors.top: reasonSection.bottom
            topPadding: 10
            bottomPadding: 10
            x: 10
            width: parent.width - 20
            font.pointSize: guiSettings.scaledFont(9/8)
            font.bold: true
            elide: Text.ElideRight
            text: qsTr("4. Details")
        }

        Rectangle {
            anchors.top: detailsHeader.bottom
            x: 10
            width: parent.width - 20
            height: detailsText.height
            radius: guiSettings.radius
            border.width: detailsText.activeFocus ? 1 : 0
            border.color: guiSettings.buttonColor
            color: guiSettings.textInputBackgroundColor
            visible: page.reasonType !== QEnums.REPORT_REASON_TYPE_NULL

            SkyFormattedTextEdit {
                id: detailsText
                width: parent.width
                topPadding: 10
                bottomPadding: 10
                parentPage: page
                parentFlick: flick
                placeholderText: qsTr("Additional details (optional)")
                maxLength: reportUtils.REPORT_DETAILS_SIZE
            }
        }
    }

    ReportUtils {
        id: reportUtils
        skywalker: page.skywalker // qmllint disable missing-type

        onReportOk: {
            skywalker.showStatusMessage(qsTr("Report sent. Thank you for your report!"), QEnums.STATUS_LEVEL_INFO)
            page.closed()
        }

        onReportFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    VirtualKeyboardHandler {
        id: keyboardHandler
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
        else if (!starterPack.isNull()) {
            reportUtils.reportPostOrFeed(starterPack.uri, starterPack.cid, reasonType, details, labelerDid)
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
        if (!starterPack.isNull())
            return "Report starter pack"

        console.warn("UKNOWN REPORT")
        return "Report"
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
