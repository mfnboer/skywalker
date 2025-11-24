import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    property int years: root.getSkywalker().getAnniversary().getAnniversaryYears()

    signal canceled
    signal addCard(string source, int years)

    id: page
    width: parent.width
    height: parent.height

    header: SimpleHeader {
        backIsCancel: true
        text: qsTr("Anniversary Card");
        onBack: {
            anniversaryCard.dropCard()
            page.canceled()
        }

        SvgPlainButton {
            id: addCardButton
            anchors.rightMargin: 10
            anchors.right: parent.right
            anchors.top: parent.top
            svg: SvgOutline.check
            accessibleName: qsTr("add card")
            onClicked: addCard(anniversaryCard.imageSource, page.years)
        }
    }

    Image {
        id: cardImage
        x: 10
        y: 10
        width: parent.width - 20
        fillMode: Image.PreserveAspectFit
        source: anniversaryCard.imageSource
    }

    ColumnLayout {
        id: colorColumn
        x: 10
        anchors.top: cardImage.bottom
        anchors.topMargin: 10
        width: parent.width - 20

        AccessibleText {
            Layout.fillWidth: true
            font.bold: true
            text: qsTr("Choose your colors")
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            SkyButton {
                id: backgroundButton
                Layout.preferredWidth: 100
                Material.background: anniversaryCard.backgroundColor
                onClicked: selectColor(anniversaryCard.backgroundColor, (color) => anniversaryCard.backgroundColor = color)
            }

            AccessibleText {
                Layout.fillWidth: true
                height: backgroundButton.height
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                text: qsTr("Background")
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            SkyButton {
                id: logoButton
                Layout.preferredWidth: 100
                Material.background: anniversaryCard.logoColor
                onClicked: selectColor(anniversaryCard.logoColor, (color) => anniversaryCard.logoColor = color)
            }

            AccessibleText {
                Layout.fillWidth: true
                height: logoButton.height
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                text: qsTr("Logo")
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            SkyButton {
                id: ageButton
                Layout.preferredWidth: 100
                Material.background: anniversaryCard.ageColor
                onClicked: selectColor(anniversaryCard.ageColor, (color) => anniversaryCard.ageColor = color)
            }

            AccessibleText {
                Layout.fillWidth: true
                height: ageButton.height
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                text: qsTr("Number")
            }
        }
    }

    SkyButton {
        text: qsTr("OK")
        anchors.top: colorColumn.bottom
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        onClicked: addCard(anniversaryCard.imageSource, page.years)
    }

    AnniversaryCard {
        id: anniversaryCard
        years: page.years
    }

    function selectColor(color, setColor) {
        let component = guiSettings.createComponent("ColorSelector.qml")
        let cs = component.createObject(page)
        cs.selectedColor = color
        cs.onRejected.connect(() => addCardButton.enabled = true)
        cs.onAccepted.connect(() => {
            addCardButton.enabled = true
            setColor(cs.selectedColor)
        })
        addCardButton.enabled = false
        cs.open()
    }
}
