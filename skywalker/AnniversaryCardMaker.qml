import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    property int years: root.getSkywalker().getAnniversaryYears()

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

        SvgButton {
            id: addCardButton
            anchors.rightMargin: 10
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            svg: svgOutline.check
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
        x: 10
        anchors.top: cardImage.bottom
        anchors.topMargin: 10
        width: parent.width - 20

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            RoundButton {
                id: backgroundButton
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

            RoundButton {
                id: logoButton
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

            RoundButton {
                id: ageButton
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

    AnniversaryCard {
        id: anniversaryCard
        years: page.years
    }

    function selectColor(color, setColor) {
        let component = Qt.createComponent("ColorSelector.qml")
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
