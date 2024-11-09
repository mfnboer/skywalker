import QtQuick
import QtQuick.Layouts
import skywalker

Column {
    property listview list
    property bool showCloseButton: false

    signal closeClicked

    id: quoteColumn
    padding: 10

    Accessible.role: Accessible.StaticText
    Accessible.name: accessibilityUtils.getListSpeech(list)

    RowLayout {
        width: parent.width - 20

        ListAvatar {
            id: avatar
            width: 34
            Layout.alignment: Qt.AlignTop
            avatarUrl: list.avatarThumb

            onClicked: root.viewListByUri(list.uri, false)
        }

        Column {
            Layout.fillWidth: true

            SkyCleanedText {
                width: parent.width
                elide: Text.ElideRight
                font.bold: true
                color: GuiSettings.textColor
                plainText: list.name

                Accessible.ignored: true
            }

            Text {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: GuiSettings.scaledFont(7/8)
                color: GuiSettings.handleColor
                text: qsTr(`${(GuiSettings.listTypeName(list.purpose))} by @${list.creator.handle}`)

                Accessible.ignored: true
            }
        }

        SvgButton {
            Layout.preferredWidth: 34
            Layout.preferredHeight: 34
            svg: SvgOutline.close
            accessibleName: qsTr("remove quoted list")
            focusPolicy: Qt.NoFocus
            visible: showCloseButton
            onClicked: closeClicked()
        }
    }

    Text {
        width: parent.width - 20
        wrapMode: Text.Wrap
        maximumLineCount: 5
        elide: Text.ElideRight
        color: GuiSettings.textColor
        text: list.description
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

}
