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
            avatarUrl: list.avatar

            onClicked: root.viewListByUri(list.uri, false)
        }

        Column {
            Layout.fillWidth: true

            SkyCleanedText {
                width: parent.width
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                plainText: list.name

                Accessible.ignored: true
            }

            Text {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: qsTr(`${(guiSettings.listTypeName(list.purpose))} by @${list.creator.handle}`)

                Accessible.ignored: true
            }
        }

        SvgButton {
            Layout.preferredWidth: 34
            Layout.preferredHeight: 34
            svg: svgOutline.close
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
        color: guiSettings.textColor
        text: list.description
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

    GuiSettings {
        id: guiSettings
    }
}
