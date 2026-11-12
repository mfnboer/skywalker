import QtQuick
import QtQuick.Controls
import skywalker

SkyHighlightRectangle {
    property Skywalker skywalker: root.getSkywalker()
    required property var ozoneEntity // reportView or subjectViewDetail
    required property basicprofile author
    property string authorError
    property bool showSubjectType: false
    readonly property int margin: 10

    id: record
    height: contentCol.height

    Column {
        id: contentCol
        width: parent.width
        spacing: 0

        AccessibleText {
            padding: margin
            width: parent.width
            font.bold: true
            text: ozoneUtils.subjectTypeToString(ozoneEntity)
            visible: showSubjectType
        }

        PostHeaderWithAvatar {
            x: margin
            bottomPadding: margin
            topPadding: showSubjectType ? 0 : margin
            width: parent.width - 2 * margin
            userDid: skywalker.getUserDid()
            author: record.author
            visible: !record.author.isNull()
        }

        AccessibleText {
            x: margin
            bottomPadding: margin
            topPadding: showSubjectType ? 0 : margin
            width: parent.width - 2 * margin
            font.bold: true
            text: record.authorError
            visible: record.author.isNull()
        }

        AccessibleText {
            x: margin
            width: parent.width - 2 * margin
            wrapMode: Text.Wrap
            text: ozoneEntity.recordContent.text
            visible: !ozoneEntity.recordContent.isNull()
        }

        AccessibleText {
            width: parent.width
            leftPadding: margin
            rightPadding: margin
            bottomPadding: margin
            color: Material.color(Material.Grey)
            font.pointSize: guiSettings.scaledFont(7/8)
            text: guiSettings.dateTimeIndication(ozoneEntity.recordContent.createdAt)
            visible: !ozoneEntity.recordContent.isNull()
        }

        AccessibleText {
            x: margin
            bottomPadding: margin
            width: parent.width - 2 * margin
            color: guiSettings.linkColor
            text: qsTr(`View ${ozoneUtils.subjectTypeToString(ozoneEntity, false)}`)
            visible: mustShowViewLink()


            MouseArea {
                anchors.fill: parent
                onClicked: openSubject()
            }
        }
    }

    OzoneUtils {
        id: ozoneUtils
        skywalker: record.skywalker
    }

    function mustShowViewLink() {
        if (ozoneEntity.subjectType === QEnums.OZONE_REPORT_SUBJECT_UNKNOWN)
            return false

        if (ozoneEntity.subjectType !== QEnums.OZONE_REPORT_SUBJECT_STRONG_REF)
            return true

        return ![QEnums.REF_TYPE_ACTOR_STATUS,
                 QEnums.REF_TYPE_ACTOR_LINK,
                 QEnums.REF_TYPE_UNKNOWN].includes(ozoneUtils.subjectToRefType(ozoneEntity))
    }

    function openSubject() {
        console.debug("Open subject type:", ozoneEntity.subjectType, "subject:", ozoneEntity.subject, "subject uri:", ozoneEntity.subjectUri)

        if (ozoneEntity.subjectType === QEnums.OZONE_REPORT_SUBJECT_STRONG_REF) {
            const refType = ozoneUtils.subjectToRefType(ozoneEntity)

            if (refType === QEnums.REF_TYPE_LIST) {
                root.viewListByUri(ozoneEntity.subjectUri, false)
                return
            }

            if (refType === QEnums.REF_TYPE_FEED) {
                skywalker.getFeedGenerator(ozoneEntity.subjectUri, false)
                return
            }
        }
        else if (ozoneEntity.subjectType === QEnums.OZONE_REPORT_SUBJECT_CONVO_REF) {
            const convoRef = ozoneEntity.subject
            skywalker.chat.startConvoForConvoId(convoRef.convoId)
            return
        }
        else if (ozoneEntity.subjectType === QEnums.OZONE_REPORT_SUBJECT_MESSAGE_REF) {
            const msgRef = ozoneEntity.subject
            skywalker.chat.startConvoForConvoId(msgRef.convoId, msgRef.messageId)
        }

        if (!ozoneEntity.subjectUri)
            return

        if (ozoneEntity.subjectUri.startsWith("did:")) {
            root.openLink(ozoneEntity.subjectUri)
            return
        }

        const httpsUrl = root.getLinkUtils().toHttpsLink(ozoneEntity.subjectUri)

        if (httpsUrl)
            root.openLink(httpsUrl)
    }
}
