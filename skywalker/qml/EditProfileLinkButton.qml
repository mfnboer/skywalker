import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyDialog {
    property Skywalker skywalker: root.getSkywalker()
    property profilelinkview linkView
    property string linkIcon: linkView.icon
    property bool initalized: false

    id: page
    width: parent.width
    title: linkView.url ? qsTr("Update link") : qsTr("Add link")
    contentHeight: linkColumn.height
    topMargin: guiSettings.headerHeight

    footer: DialogButtonBox {
        SkyTransparentButton {
            text: qsTr("Cancel")
            onClicked: page.reject()
        }
        SkyTransparentButton {
            id: removeButton
            text: qsTr("Remove")
            onClicked: confirmDelete()
        }
        SkyTransparentButton {
            text: qsTr("OK")
            onClicked: page.handleOk()
            enabled: linkField.displayText && linkField.isValid() && !busyIndicator.running
        }

        Component.onCompleted: {
            if (!linkView.url)
                removeItem(removeButton)
        }
    }

    ColumnLayout {
        id: linkColumn
        width: parent.width
        spacing: 5

        AccessibleText {
            Layout.fillWidth: true
            font.bold: true
            text: qsTr("Link")
        }

        SkyWebLinkInput {
            id: linkField
            Layout.fillWidth: true
            padding: 5
            focus: true
            initialText: page.linkView.url
            placeholderText: qsTr("example.com")
            onDisplayTextChanged: {
                if (!displayText) {
                    titleField.text = ""
                    linkIcon = ""
                    favoIconTimer.stop()
                }
                else if (linkField.isValid()) {
                    favoIconTimer.startForLink(getLink())
                }
            }
        }

        AccessibleText {
            Layout.fillWidth: true
            font.bold: true
            text: qsTr("Title (optional)")
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: titleField.height
            radius: guiSettings.radius
            border.width: titleField.activeFocus ? 1 : 0
            border.color: guiSettings.buttonColor
            color: guiSettings.textInputBackgroundColor

            SkyTextInput {
                id: titleField
                width: parent.width
                initialText: page.linkView.title
                placeholderText: qsTr(`Title to show on button`)
                maximumGraphemeLength: 30
                strictGraphemeMax: true
            }
        }

        AccessibleText {
            Layout.fillWidth: true
            font.bold: true
            text: qsTr("Preview")
        }

        ProfileLinkButton {
            id: linkButton
            linkView: linkCardReader.makeProfileLinkView(page.linkView.uri, page.linkView.cid, linkField.getLink(), titleField.text, page.linkIcon)
            visibility: QEnums.CONTENT_VISIBILITY_SHOW
            onClicked: skywalker.showStatusMessage(qsTr("It works!"), QEnums.STATUS_LEVEL_INFO)

            BusyIndicator {
                id: busyIndicator
                x: 5
                width: height
                height: parent.height
                running: false
            }
        }
    }

    LinkCardReader {
        id: linkCardReader
        skywalker: page.skywalker

        onFavIcon: (linkView) => {
            busyIndicator.running = false

            if (!titleField.text)
                titleField.text = linkView.title

            page.linkIcon = linkView.icon
        }

        onFavIconFailed: busyIndicator.running = false
    }

    Timer {
        property string webLink

        id: favoIconTimer
        interval: 1000
        onTriggered: {
            busyIndicator.running = true
            linkCardReader.getFavIcon(webLink)
        }

        function startForLink(webLink) {
            if (!initalized)
                return

            favoIconTimer.webLink = webLink
            start()
        }
    }

    function handleOk() {
        console.debug("old:", linkView)
        console.debug("new:", linkButton.linkView)

        if (linkView.url == linkButton.linkView.url &&
                linkView.title == linkButton.linkView.title &&
                linkView.icon == linkButton.linkView.icon)
        {
            page.reject()
        }
        else
        {
            page.accept()
        }
    }

    function confirmDelete() {
        guiSettings.askYesNoQuestion(
                    page,
                    qsTr("Do you really want to remove this link?"),
                    () => page.discarded())
    }

    function getLinkView() {
        return linkButton.linkView
    }

    Component.onCompleted: {
        initalized = true
        linkField.setFocus()
    }
}
