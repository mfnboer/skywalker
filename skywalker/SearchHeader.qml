import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Rectangle {
    property int minSearchTextLength: 1
    property string placeHolderText: qsTr("Search")
    property bool showSearchButton: true
    property bool showBackButton: true
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
        y: guiSettings.headerMargin
        width: parent.width
        height: guiSettings.headerHeight - guiSettigs.headerMargin

        SvgPlainButton {
            id: backButton
            iconColor: guiSettings.headerTextColor
            svg: SvgOutline.arrowBack
            accessibleName: qsTr("go back")
            visible: showBackButton
            onClicked: headerRect.back()
        }

        Rectangle {
            radius: 5
            Layout.fillWidth: true
            Layout.rightMargin: 10
            Layout.preferredHeight: searchText.height
            color: guiSettings.textInputBackgroundColor
            border.width: searchText.activeFocus ? 1 : 0
            border.color: guiSettings.buttonColor

            TextInput {
                id: searchText
                EnterKey.type: Qt.EnterKeySearch // works since Qt6.8
                width: parent.width
                clip: true
                padding: 5
                rightPadding: clearButton.width
                font.pointSize: guiSettings.scaledFont(9/8)
                color: guiSettings.textColor
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                maximumLength: 2048
                focus: true

                onDisplayTextChanged: {
                    if (displayText !== prevDisplayText) {
                        prevDisplayText = displayText
                        headerRect.searchTextChanged(displayText)
                    }
                }

                Keys.onReturnPressed: headerRect.search(searchText.displayText)

                Accessible.role: Accessible.EditableText
                Accessible.name: placeHolder.visible ? placeHolderText : text
                Accessible.editable: true
                Accessible.searchEdit: true

                SvgPlainButton {
                    id: clearButton
                    anchors.right: parent.right
                    imageMargin: 8
                    y: parent.y - parent.padding
                    width: height
                    height: parent.height + 10
                    svg: SvgOutline.close
                    Material.background: guiSettings.backgroundColor
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
                color: guiSettings.placeholderTextColor
                text: headerRect.placeHolderText
                visible: searchText.displayText.length === 0

                Accessible.ignored: true
            }
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
