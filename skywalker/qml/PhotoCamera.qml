import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import QtMultimedia
import skywalker

SkyPage {
    property Skywalker skywalker: root.getSkywalker()
    readonly property string sideBarTitle: qsTr("Photo camera")
    readonly property bool noSideBar: true

    signal closed
    signal captured(string fileName)

    id: page
    width: root.width
    height: root.height
    background: Rectangle { color: guiSettings.fullScreenColor }

    SvgButton {
        x: guiSettings.leftMargin
        y: guiSettings.headerMargin
        iconColor: "white"
        Material.background: guiSettings.fullScreenColor
        opacity: 0.7
        svg: SvgOutline.arrowBack
        accessibleName: qsTr("go back")
        onClicked: page.closed()
    }

    MediaDevices {
        id: mediaDevices
    }

    CaptureSession {
        imageCapture : ImageCapture {
            id: imageCapture

            onImageSaved: (id, fileName) => {
                console.debug("Image saved:", id, "file:", fileName)
                page.captured(fileName)
            }

            onErrorOccurred: (id, error, errorString) => {
                console.warn("Capture error:", id, error, errorString)
                skywalker.showStatusMessage(errorString, QEnums.STATUS_LEVEL_ERROR)
            }
        }
        camera: Camera {
            id: camera
            cameraDevice: mediaDevices.defaultVideoInput
            active: true

            onErrorChanged: {
                if (error == Camera.CameraError) {
                    console.warn("Camera error:", errorString)
                    skywalker.showStatusMessage(errorString, QEnums.STATUS_LEVEL_ERROR)
                }
            }
        }

        videoOutput: videoOutput
    }

    VideoOutput {
        id: videoOutput
        anchors.fill: parent

        RoundButton {
            id: photoButton
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10 + guiSettings.footerMargin
            width: 70
            height: width
            Material.background: "white"
            enabled: camera.error == Camera.NoError

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("take picture")
            Accessible.onPressAction: photoButton.clicked()

            onClicked: page.capturePhoto()
        }
    }

    MultiEffect {
        id: blur
        source: videoOutput
        anchors.fill: videoOutput
        blurEnabled: true
        blur: 1.0
        blurMax: 64
        visible: false

        transform: Rotation {
            id: pageRotation
            origin.x: page.width / 2
            origin.y: page.height / 2
            axis { x: 1; y: 0; z: 0 }
            angle: 0
        }

        function activate(active) {
            visible = active
            videoOutput.visible = !active
        }
    }

    DragHandler {
        property real startY: 0

        target: null

        onActiveChanged: {
            if (active) {
                startY = centroid.position.y
            } else {
                const deltaY = centroid.position.y - startY

                if (deltaY > 90)
                    toggleCamera(true)
                else if (deltaY < -90)
                    toggleCamera(false)
            }
        }
    }

    Rectangle {
        id: flash
        anchors.fill: parent
        color: guiSettings.fullScreenColor
        visible: false
    }

    Timer {
        id: flashTimer
        interval: 200

        onTriggered: flash.visible = false

        function go() {
            flash.visible = true
            start()
        }
    }

    SoundEffect {
        id: cameraShutter
        audioDevice: mediaDevices.defaultAudioOutput
        source: "/sounds/camera-shutter-click.wav"
        volume: 1.0
        loops: 1
        onStatusChanged: console.debug("SOUND:", status)
        onPlayingChanged: console.debug("PLAYING:", playing)
    }

    NumberAnimation {
        property var doneCb

        id: rotationAnimation
        target: pageRotation
        property: "angle"
        duration: 300

        onFinished: doneCb()

        function go(fromAngle, toAngle, easType, cb = () => {}) {
            doneCb = cb
            from = fromAngle
            to = toAngle
            easing.type = easType
            start()
        }
    }

    CameraUtils {
        id: cameraUtils
    }

    function toggleCamera(downSwipe) {
        console.debug("Toggle camera")
        let position = null

        if (camera.cameraDevice.position == CameraDevice.BackFace)
            position = CameraDevice.FrontFace
        else if (camera.cameraDevice.position == CameraDevice.FrontFace)
            position = CameraDevice.BackFace

        if (position == null) {
            console.warn("Cannot toggle camera:", camera.cameraDevice.position)
            return
        }

        for (const cam of mediaDevices.videoInputs) {
            if (cam.position === position) {
                blur.activate(true)
                const dir = downSwipe ? 1 : -1

                rotationAnimation.go(0, -90 * dir, Easing.InCubic, () => {
                    rotationAnimation.go(-270 * dir, -360 * dir, Easing.OutCubic, () => { blur.activate(false) })
                })

                camera.cameraDevice = cam
                return
            }
        }

        console.warn("No camera available for:", position)
    }

    function capturePhoto() {
        cameraShutter.play()
        flashTimer.go()
        const fileName = cameraUtils.getCaptureFileName()

        if (!fileName) {
            skywalker.showStatusMessage(qsTr("Cannot save file"), QEnums.STATUS_LEVEL_ERROR)
            return
        }

        console.debug("Capture to:", fileName)
        imageCapture.captureToFile(fileName)
    }

    function setSystemBarsColor() {
        displayUtils.setNavigationBarColorAndMode("transparent", false)
        displayUtils.setStatusBarTransparentAndMode(true, guiSettings.fullScreenColor, false)
    }

    function resetSystemBarsColor() {
        displayUtils.setNavigationBarColor(guiSettings.backgroundColor)
        displayUtils.setStatusBarTransparent(false, guiSettings.headerColor)
    }

    function cancel() {
        closed()
    }

    Component.onDestruction: {
        resetSystemBarsColor()
    }

    Component.onCompleted: {
        setSystemBarsColor()
        console.debug("Camera:", camera.cameraDevice)

        for (const dev of mediaDevices.videoInputs) {
            console.debug("Video input:", dev)
        }
    }
}
