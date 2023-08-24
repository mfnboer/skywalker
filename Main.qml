import QtQuick
import QtQuick.Controls.Material
import QtQuick.Window
import skywalker

Window {
    width: 480
    height: 960
    visible: true
    title: qsTr("Skywalker")

    Login {
        id: loginDialog
        anchors.centerIn: parent
        onAccepted: skywalker.login(user, password, host)

    }

    Skywalker {
        id: skywalker
        onLoginFailed: (error) => loginDialog.show(error)
    }

    Component.onCompleted: {
        loginDialog.show()
    }
}
