import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

// Header with a back button and title text
Rectangle {
    required property string text
    property string subTitle
    property bool backIsCancel: false
    property bool headerVisible: true
    property bool isSideBar: false
    property string userDid
    readonly property int usedRightMargin: currentUserAvatar.active ? currentUserAvatar.width : 0

    signal back

    id: headerRect
    width: parent.width
    height: visible ? guiSettings.headerHeight : 0
    z: guiSettings.headerZLevel
    color: guiSettings.headerColor

    Accessible.role: Accessible.Pane

    RowLayout
    {
        id: headerRow
        y: !isSideBar ? guiSettings.headerMargin : 0
        width: parent.width
        visible: headerVisible
        Accessible.role: Accessible.Pane

        SvgPlainButton {
            id: backButton
            svg: backIsCancel ? SvgOutline.cancel : SvgOutline.arrowBack
            accessibleName: backIsCancel ? qsTr("cancel") : qsTr("go back")
            onClicked: headerRect.back()
        }

        Column {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            Text {
                width: parent.width
                font.bold: true
                font.pointSize: (Boolean(subTitle) || isSideBar) ? guiSettings.scaledFont(1) : guiSettings.scaledFont(10/8)
                color: guiSettings.headerTextColor
                elide: Text.ElideRight
                text: headerRect.text

                Accessible.role: Accessible.TitleBar
                Accessible.name: text
                Accessible.description: Accessible.name
            }
            AccessibleText {
                width: parent.width
                color: guiSettings.handleColor
                font.pointSize: guiSettings.scaledFont(7/8)
                text: subTitle
                visible: Boolean(subTitle)
            }
        }

        Loader {
            id: currentUserAvatar
            Layout.rightMargin: 10
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            Layout.preferredHeight: parent.height - 10
            Layout.preferredWidth: Layout.preferredHeight
            active: !root.isActiveUser(headerRect.userDid)

            sourceComponent: CurrentUserAvatar {
                userDid: headerRect.userDid
            }
        }
    }

}
