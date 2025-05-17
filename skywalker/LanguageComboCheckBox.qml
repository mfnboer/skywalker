import QtQuick
import QtQuick.Controls
import skywalker

ComboBox {
    property list<language> allLanguages
    property list<language> usedLanguages
    property list<string> checkedLangCodes
    property bool noneCheckedMeansAll: true
    property int radius: 0
    property string borderColor: guiSettings.buttonColor
    property string color: guiSettings.buttonColor

    id: languageComboBox
    height: 34
    model: usedLanguages.concat(allLanguages)
    valueRole: "shortCode"
    textRole: "shortCode"
    popup.width: 250

    background: Rectangle {
        implicitWidth: 46
        implicitHeight: 34
        radius: languageComboBox.radius
        border.color: languageComboBox.borderColor
        border.width: 1
        color: "transparent"
    }

    indicator: Item {}

    contentItem: Text {
        leftPadding: 10
        rightPadding: 10
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        color: languageComboBox.color
        text: checkedLangCodes.length > 0 ? checkedLangCodes.join(", ") : (noneCheckedMeansAll ? qsTr("all languages") : qsTr("none"))
    }

    delegate: CheckDelegate {
        required property int index
        required property language modelData

        id: delegate
        width: popup.width
        checked: checkedLangCodes.includes(modelData.shortCode)
        highlighted: languageComboBox.highlightedIndex === index

        indicator: Rectangle {
            x: delegate.leftPadding
            y: (delegate.height - height) / 2
            implicitWidth: 20
            implicitHeight: 20
            color: "transparent"
            border.color: guiSettings.textColor

            Rectangle {
                width: 14
                height: 14
                x: 3
                y: 3
                color: guiSettings.textColor
                visible: delegate.checked
            }
        }

        contentItem: Text {
            leftPadding: delegate.indicator.width + delegate.spacing
            width: delegate.width
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            color: guiSettings.textColor
            text: `${delegate.modelData.nativeName} (${delegate.modelData.shortCode})`
        }

        background: Rectangle {
            implicitWidth: delegate.width
            color: delegate.highlighted ? Material.listHighlightColor : (delegate.index < usedLanguages.length ? guiSettings.postHighLightColor : "transparent")
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                const newCode = delegate.modelData.shortCode

                if (delegate.checked) {
                    const index = checkedLangCodes.indexOf(newCode)
                    checkedLangCodes.splice(index, 1)
                }
                else {
                    const index = checkedLangCodes.findIndex((code) => { return code > newCode })

                    if (index === -1)
                        checkedLangCodes.push(newCode)
                    else
                        checkedLangCodes.splice(index, 0, newCode)
                }
            }
        }
    }

    popup.contentItem: ListView {
        clip: true
        implicitHeight: contentHeight
        model: languageComboBox.popup.visible ? languageComboBox.delegateModel : null
        currentIndex: languageComboBox.highlightedIndex
        ScrollIndicator.vertical: ScrollIndicator {}

        footer: Rectangle {
            width: parent.width
            height: guiSettings.footerHeight
            color: languageComboBox.popup.background.color
            z: guiSettings.footerZLevel

            SkyButton {
                anchors.centerIn: parent
                width: parent.width - 20
                text: qsTr("OK")
                onClicked: languageComboBox.popup.close()
            }
        }

        footerPositioning: ListView.OverlayFooter
    }

}
