import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

ListView {
    required model
    required property string postUri
    required property int action // QEnums::NonActiveUserAction
    property int rowPadding: 2

    signal userClicked(NonActiveUser user)
    signal repostClicked(NonActiveUser user)
    signal quoteClicked(NonActiveUser user)

    id: listView
    spacing: 0
    boundsBehavior: Flickable.StopAtBounds
    clip: true
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned

    Accessible.role: Accessible.List

    delegate: Rectangle {
        required property NonActiveUser modelData
        property alias user: userEntry.modelData

        id: userEntry
        width: listView.width
        height: grid.height
        color: guiSettings.backgroundColor

        Accessible.role: Accessible.Button
        Accessible.name: user.profile.name
        Accessible.onPressAction: {
            if (action !== QEnums.NON_ACTIVE_USER_REPOST)
                userClicked(user)
        }

        GridLayout {
            id: grid
            width: parent.width
            columns: 3
            rowSpacing: 0
            columnSpacing: 10

            // Avatar
            Rectangle {
                id: avatar
                Layout.rowSpan: 2
                Layout.preferredWidth: 44
                Layout.preferredHeight: avatarImg.height
                Layout.fillHeight: true
                color: "transparent"

                Accessible.ignored: true

                Avatar {
                    id: avatarImg
                    anchors.centerIn: parent
                    width: parent.width - 12
                    author: userEntry.user.profile
                    onClicked: {
                        if (action !== QEnums.NON_ACTIVE_USER_REPOST)
                            userClicked(userEntry.user)
                    }
                }
            }

            AuthorNameAndStatus {
                Layout.topMargin: rowPadding
                Layout.fillWidth: true
                author: user.profile
            }

            NonActiveUserActionIcon {
                id: expiredIcon
                svg: SvgOutline.warning
                iconColor: guiSettings.disabledColor
                visible: user.sessionExpired
            }

            NonActiveUserActionIcon {
                id: notFoundIcon
                svg: SvgOutline.block
                iconColor: guiSettings.disabledColor
                visible: !user.sessionExpired && Boolean(user.postView) &&
                         user.postView.uri === postUri && user.postView.notFound
            }

            NonActiveUserActionIcon {
                id: errorIcon
                svg: SvgOutline.error
                iconColor: guiSettings.errorColor
                visible: !user.sessionExpired && Boolean(user.postView) &&
                         user.postView.uri === postUri && user.postView.hasError()
            }

            NonActiveUserActionIcon {
                id: actionIcon
                svg: actionSvg()
                iconColor: actionIconColor()
                visible: actionVisible() && !user.sessionExpired &&
                         Boolean(user.postView) && user.postView.uri === postUri &&
                         user.postView.isGood()

                BlinkingOpacity {
                    target: actionIcon
                    running: user.actionInProgress
                }

                function actionVisible() {
                    switch (action) {
                    case QEnums.NON_ACTIVE_USER_LIKE:
                    case QEnums.NON_ACTIVE_USER_BOOKMARK:
                    case QEnums.NON_ACTIVE_USER_REPLY:
                        return true
                    }

                    return false
                }

                function actionSvg() {
                    switch (action) {
                    case QEnums.NON_ACTIVE_USER_LIKE:
                        return user.postView?.likeUri ? SvgFilled.like : SvgOutline.like
                    case QEnums.NON_ACTIVE_USER_BOOKMARK:
                        return user.postView?.bookmarked ? SvgFilled.bookmark : SvgOutline.bookmark
                    case QEnums.NON_ACTIVE_USER_REPLY:
                        return SvgOutline.reply
                    }

                    return SvgOutline.error
                }

                function actionIconColor() {
                    switch (action) {
                    case QEnums.NON_ACTIVE_USER_LIKE:
                        return user.postView?.likeUri ? guiSettings.likeColor : guiSettings.statsColor
                    case QEnums.NON_ACTIVE_USER_BOOKMARK:
                        return user.postView?.bookmarked ?  guiSettings.buttonColor : guiSettings.statsColor
                    case QEnums.NON_ACTIVE_USER_REPLY:
                        return user.postView?.replyDisabled ? guiSettings.disabledColor : guiSettings.statsColor
                    }

                    return guiSettings.errorColor
                }
            }

            Row {
                id: repostAction
                Layout.rowSpan: 2
                visible: action === QEnums.NON_ACTIVE_USER_REPOST && !user.sessionExpired &&
                         Boolean(user.postView) && user.postView.uri === postUri &&
                         user.postView.isGood()

                SvgButton {
                    width: 44
                    height: width
                    svg: SvgFilled.quote
                    accessibleName: qsTr("quote post")
                    enabled: !user.postView?.embeddingDisabled

                    onClicked: quoteClicked(user)
                }

                SvgButton {
                    id: repostButton
                    width: 44
                    height: width
                    flat: Boolean(user.postView?.repostUri)
                    svg: SvgOutline.repost
                    accessibleName: user.postView?.repostUri ? qsTr("undo repost") : qsTr("repost")

                    onClicked: repostClicked(user)

                    BlinkingOpacity {
                        target: repostButton
                        running: user.actionInProgress
                    }
                }
            }

            BusyIndicator {
                id: progressIcon
                Layout.preferredWidth: 44
                Layout.preferredHeight: Layout.preferredWidth
                Layout.rowSpan: 2
                running: user.getPostInProgress
                visible: running
            }

            Rectangle {
                Layout.rowSpan: 2
                Layout.preferredHeight: 44
                color: "transparent"
                visible: !expiredIcon.visible && !notFoundIcon.visible && !errorIcon.visible &&
                         !actionIcon.visible && !repostAction.visible && !progressIcon.visible
            }

            AccessibleText {
                bottomPadding: rowPadding
                Layout.fillWidth: true
                Layout.fillHeight: true
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: user.profile.handle ? `@${user.profile.handle}` : ""

                Accessible.ignored: true
            }

            Item{}

            Rectangle {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.preferredHeight: contentLabels.height + 5
                color: "transparent"

                ContentLabels {
                    id: contentLabels
                    anchors.left: parent.left
                    anchors.right: undefined
                    contentLabels: user.profile.labels
                    contentAuthor: user.profile
                }
            }

            Rectangle {
                Layout.columnSpan: 3
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: guiSettings.separatorColor
            }
        }
        MouseArea {
            z: -1
            anchors.fill: parent
            enabled: action !== QEnums.NON_ACTIVE_USER_REPOST
            onClicked: userClicked(user)
        }

        Component.onCompleted: {
            if (!user.sessionExpired && (!Boolean(user.postView) || user.postView.uri !== postUri ||
                                         !user.postView.isGood()))
            {
                user.getPost(postUri)
            }
        }
    }
}
