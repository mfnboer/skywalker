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
    height: guiSettings.headerHeight
    z: guiSettings.headerZLevel
    color: guiSettings.headerColor

    RowLayout {
        width: parent.width
        height: guiSettings.headerHeight

        SvgButton {
            id: backButton
            iconColor: guiSettings.headerTextColor
            Material.background: "transparent"
            svg: svgOutline.arrowBack
            onClicked: headerRect.back()

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("go back")
            Accessible.onPressAction: clicked()
        }

        Rectangle {
            radius: 5
            Layout.fillWidth: true
            height: searchText.height
            color: guiSettings.backgroundColor

            TextInput {
                id: searchText
                EnterKeyAction.actionId: EnterKeyAction.Search
                width: parent.width
                clip: true
                padding: 5
                rightPadding: clearButton.width
                font.pointSize: guiSettings.scaledFont(9/8)
                color: guiSettings.textColor
                inputMethodHints: Qt.ImhNoAutoUppercase
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
                    svg: svgOutline.close
                    visible: searchText.displayText.length > 0
                    onClicked: searchText.clear()

                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("clear search text")
                    Accessible.onPressAction: clicked()
                }
            }

            Text {
                id: placeHolder
                width: searchText.width
                padding: searchText.padding
                font.pointSize: searchText.font.pointSize
                color: guiSettings.placeholderTextColor
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
            iconColor: enabled ? guiSettings.headerTextColor : guiSettings.disabledColor
            Material.background: enabled ? guiSettings.buttonColor : guiSettings.headerColor
            svg: svgOutline.search
            onClicked: headerRect.search(searchText.displayText)
            enabled: searchText.displayText.length >= headerRect.minSearchTextLength
            visible: headerRect.showSearchButton

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("start search for posts and users")
            Accessible.onPressAction: if (enabled) clicked()
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function setSearchText(text) {
        searchText.text = text
    }

    function getDisplayText() {
        return searchText.displayText
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
