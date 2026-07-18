import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Rectangle {
    required property string title
    property string subTitle
    property string description
    property bool isSideBar: false
    property string userDid
    property int rightPadding: 0
    readonly property int usedRightMargin: currentUserAvatar.active ? currentUserAvatar.width : 0

    signal closed

    id: header
    width: parent.width
    height: visible ? headerColumn.height : 0
    Layout.preferredHeight: visible ? headerColumn.height : 0
    z: guiSettings.headerZLevel
    color: "transparent"

    Column {
        id: headerColumn
        width: parent.width

        Rectangle {
            width: parent.width  - header.rightPadding
            height: guiSettings.headerHeight
            color: guiSettings.headerColor

            RowLayout {
                id: headerRow
                width: parent.width
                height: guiSettings.headerHeight

                SvgPlainButton {
                    id: backButton
                    iconColor: guiSettings.headerTextColor
                    svg: SvgOutline.arrowBack
                    accessibleName: qsTr("go back")
                    onClicked: header.closed()
                }

                Column {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter

                    AccessibleText {
                        width: parent.width
                        font.bold: true
                        font.pointSize: (Boolean(subTitle) || isSideBar) ? guiSettings.scaledFont(1) : guiSettings.scaledFont(10/8)
                        elide: Text.ElideRight
                        color: guiSettings.headerTextColor
                        text: title

                        Accessible.role: Accessible.TitleBar
                        Accessible.name: text
                        Accessible.description: Accessible.name
                    }
                    AccessibleText {
                        width: parent.width
                        color: guiSettings.handleColor
                        font.pointSize: guiSettings.scaledFont(7/8)
                        elide: Text.ElideRight
                        text: subTitle
                        visible: Boolean(subTitle)
                    }
                }

                Loader {
                    id: currentUserAvatar
                    Layout.rightMargin: active ? 10 : 0
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    Layout.preferredHeight: active ? parent.height - 10 : 0
                    Layout.preferredWidth: Layout.preferredHeight
                    active: !root.isActiveUser(header.userDid)

                    sourceComponent: CurrentUserAvatar {
                        userDid: header.userDid
                    }
                }
            }
        }
        Rectangle {
            width: parent.width
            height: descriptionText.height
            color: guiSettings.backgroundColor
            visible: description

            Accessible.role: Accessible.StaticText
            Accessible.name: description
            Accessible.description: Accessible.name

            AccessibleText {
                id: descriptionText
                width: parent.width
                padding: 10
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                text: description

                Accessible.ignored: true
            }
        }
    }
}
