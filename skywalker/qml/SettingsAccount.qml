import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

GridLayout {
    required property var userPrefs
    property var skywalker: root.getSkywalker()

    columns: 2

    HeaderText {
        Layout.columnSpan: 2
        text: qsTr("Account")
    }

    AccessibleText {
        text: qsTr("Email:")
    }
    RowLayout {
        Layout.fillWidth: true

        AccessibleText {
            id: mailText
            Layout.fillWidth: true
            elide: Text.ElideRight
            text: userPrefs.email
        }
        SvgPlainButton {
            id: mailConfirmedImg
            imageMargin: 0
            implicitWidth: height
            implicitHeight: mailText.height
            iconColor: guiSettings.buttonColor
            accessibleName: qsTr("E-mail address confirmed")
            svg: SvgOutline.check
            visible: userPrefs.emailConfirmed
            onClicked: skywalker.showStatusMessage(accessibleName, QEnums.STATUS_LEVEL_INFO)
        }
        SvgPlainButton {
            imageMargin: 0
            implicitWidth: height
            implicitHeight: mailText.height
            accessibleName: qsTr("Two-factor authentication enabled")
            svg: SvgOutline.confirmationCode
            visible: userPrefs.emailAuthFactor
            onClicked: skywalker.showStatusMessage(accessibleName, QEnums.STATUS_LEVEL_INFO)
        }
    }

    AccessibleText {
        color: guiSettings.textColor
        text: qsTr("Birthday:")
    }
    AccessibleText {
        Layout.fillWidth: true
        color: guiSettings.textColor
        text: userPrefs.birthDate
    }

    AccessibleText {
        color: guiSettings.textColor
        text: "PDS:"
    }
    AccessibleText {
        Layout.fillWidth: true
        color: guiSettings.textColor
        elide: Text.ElideRight
        text: userPrefs.pds
    }

    AccessibleText {
        id: didLabel
        color: guiSettings.textColor
        text: "DID:"
    }
    RowLayout {
        Layout.fillWidth: true

        AccessibleText {
            Layout.fillWidth: true
            color: guiSettings.textColor
            elide: Text.ElideRight
            text: userPrefs.did
        }
        SvgPlainButton {
            imageMargin: 0
            implicitWidth: height
            implicitHeight: didLabel.height
            svg: SvgOutline.copy
            accessibleName: qsTr("copy") + " D I D"
            onClicked: skywalker.copyToClipboard(userPrefs.did)
        }
    }

    HeaderText {
        Layout.columnSpan: 2
        Layout.topMargin: 10
        text: qsTr("Logged-out visibility")
    }

    CheckBox {
        id: loggedoutSwitch
        Layout.columnSpan: 2
        Layout.fillWidth: true

        contentItem: Text {
            text: qsTr("Discourage apps from showing my account to logged-out users");
            color: guiSettings.textColor
            wrapMode: Text.Wrap
            verticalAlignment: Text.AlignVCenter
            anchors.left: loggedoutSwitch.indicator.right
            anchors.leftMargin: loggedoutSwitch.spacing
            anchors.right: parent.right
            anchors.rightMargin: loggedoutSwitch.rightPadding
        }

        checked: !userPrefs.loggedOutVisibility
        onCheckedChanged: userPrefs.loggedOutVisibility = !checked

        Accessible.role: Accessible.Button
        Accessible.name: contentItem.text // qmllint disable missing-property
        Accessible.onPressAction: toggle()

        Component.onCompleted: {
            loggedoutSwitch.indicator.x = loggedoutSwitch.leftPadding
        }
    }
}
