import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.VirtualKeyboard
import skywalker

Rectangle {
    property int minSearchTextLength: 1
    property string placeHolderText: qsTr("Search")
    property bool showSearchButton: true
    property string prevDisplayText

    signal back
    signal searchTextChanged(string text)
    signal keyRelease(var event)
    signal search(string text)

    id: headerRect
    width: parent.width
    height: GuiSettings.headerHeight
    z: GuiSettings.headerZLevel
    color: GuiSettings.headerColor

    RowLayout {
        width: parent.width
        height: GuiSettings.headerHeight

        SvgButton {
            id: backButton
            iconColor: GuiSettings.headerTextColor
            Material.background: "transparent"
            svg: SvgOutline.arrowBack
            accessibleName: qsTr("go back")
            onClicked: headerRect.back()
        }

        Rectangle {
            radius: 5
            Layout.fillWidth: true
            height: searchText.height
            color: GuiSettings.backgroundColor

            TextInput {
                id: searchText
                EnterKeyAction.actionId: EnterKeyAction.Search
                // TODO in Qt6.8 EnterKey.type: Qt.EnterKeySearch
                width: parent.width
                clip: true
                padding: 5
                rightPadding: clearButton.width
                font.pointSize: GuiSettings.scaledFont(9/8)
                color: GuiSettings.textColor
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                maximumLength: 2048
                focus: true

                onDisplayTextChanged: {
                    if (displayText !== prevDisplayText) {
                        prevDisplayText = displayText
                        headerRect.searchTextChanged(displayText)
                    }
                }

                // Does not work with Android
                Keys.onReleased: (event) => { headerRect.keyRelease(event) }

                Accessible.role: Accessible.EditableText
                Accessible.name: placeHolder.visible ? placeHolderText : text
                Accessible.editable: true
                Accessible.searchEdit: true

                SvgButton {
                    id: clearButton
                    anchors.right: parent.right
                    imageMargin: 8
                    y: parent.y - parent.padding
                    width: height
                    height: parent.height + 10
                    svg: SvgOutline.close
                    accessibleName: qsTr("clear search text")
                    visible: searchText.displayText.length > 0
                    onClicked: searchText.clear()
                }
            }

            Text {
                id: placeHolder
                width: searchText.width
                padding: searchText.padding
                font.pointSize: searchText.font.pointSize
                color: GuiSettings.placeholderTextColor
                text: headerRect.placeHolderText
                visible: searchText.displayText.length === 0

                Accessible.ignored: true
            }
        }

        // WORKAROUND for Android
        // Qt does not catch the signal of the ENTER key from the Android
        // keyboard, nor can it set the icon to a search icon.
        SvgButton {
            id: searchButton
            iconColor: enabled ? GuiSettings.headerTextColor : GuiSettings.disabledColor
            Material.background: enabled ? GuiSettings.buttonColor : GuiSettings.headerColor
            svg: SvgOutline.search
            accessibleName: qsTr("start search for posts and users")
            onClicked: headerRect.search(searchText.displayText)
            enabled: searchText.displayText.length >= headerRect.minSearchTextLength
            visible: headerRect.showSearchButton
        }
    }


    function setSearchText(text) {
        searchText.text = text
    }

    function getDisplayText() {
        return searchText.displayText
    }

    function hasFocus() {
        return searchText.activeFocus
    }

    function forceFocus() {
        searchText.forceActiveFocus()
    }

    function unfocus() {
        // Hack to hide the selection anchor on Android that should not have been
        // there at all.
        backButton.forceActiveFocus()
    }
}
