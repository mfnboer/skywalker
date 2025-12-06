import QtQuick
import skywalker

SkyPage {
    readonly property bool noSideBar: true

    signal closed()

    width: parent.width
    height: parent.height
    background: Rectangle { color: guiSettings.skywalkerLogoColor }

    Accessible.role: Accessible.Pane

    Column {
        width: parent.width
        height: parent.height

        AccessibleText {
            id: title
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10 + guiSettings.headerMargin
            color: "white"
            font.bold: true
            font.pointSize: guiSettings.absScaledFont(3.5)
            text: skywalker.APP_NAME
        }
        AccessibleText {
            id: version
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10
            color: "white"
            text: qsTr("Version") + ": " + skywalker.VERSION
        }
        AccessibleText {
            id: author
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10
            color: "white"
            text: "\u00A9 2025 Michel de Boer"
        }
        AccessibleText {
            id: handle
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10
            textFormat: Text.RichText
            text: `<a href=\"did:plc:zzmeflm2wzrrgcaam6bw3kaf\" style=\"color: ivory; text-decoration: none\">${guiSettings.skywalkerHandle}</a>`
            onLinkActivated: (link) => skywalker.getDetailedProfile(link)

            Accessible.role: Accessible.Link
            Accessible.name: `${guiSettings.skywalkerHandle}`
            Accessible.onPressAction: skywalker.getDetailedProfile("did:plc:zzmeflm2wzrrgcaam6bw3kaf")
        }
        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            width: height
            height: Math.min(parent.height - title.height - version.height - author.height - handle.height - warranty.height - okButton.height - whitespace.height, parent.width)
            fillMode: Image.PreserveAspectFit
            source: "/images/skywalker.png"

            Accessible.ignored: true
        }
        AccessibleText {
            id: warranty
            width: parent.width
            padding: 10
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            color: "white"
            text: qsTr("Skywalker comes with ABSOLUTELY NO WARRANTY.")
        }
        SkyButton {
            id: okButton
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("OK")
            onClicked: closed()
        }
        Rectangle {
            id: whitespace
            width: 10
            height: 10
            color: "transparent"
        }
    }

    Component.onDestruction: {
        displayUtils.setNavigationBarColor(guiSettings.backgroundColor)
        displayUtils.setStatusBarColor(guiSettings.headerColor)
    }

    Component.onCompleted: {
        displayUtils.setNavigationBarColorAndMode(guiSettings.skywalkerLogoColor, false)
        displayUtils.setStatusBarColorAndMode(guiSettings.skywalkerLogoColor, false)
    }
}
