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

    MediaDevices {
        id: mediaDevices
    }

    CaptureSession {
        imageCapture : ImageCapture {
            id: imageCapture

            onImageCaptured: shutter.go()

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
            property bool hasFlash: getHasFlash()

            id: camera
            cameraDevice: mediaDevices.defaultVideoInput
            active: true

            onCameraDeviceChanged: hasFlash = getHasFlash()

            onErrorOccurred: (error, errorString) => {
                console.warn("Camera error:", errorString)
                skywalker.showStatusMessage(errorString, QEnums.STATUS_LEVEL_ERROR)
            }

            function getHasFlash() {
                return isFlashModeSupported(Camera.FlashOn) || isFlashModeSupported(Camera.FlashAuto)
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
            width: 80
            height: width
            Material.background: "white"

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("take picture")
            Accessible.onPressAction: photoButton.clicked()

            onClicked: page.capturePhoto()
        }

        SvgButton {
            id: cameraButton
            anchors.left: photoButton.right
            anchors.leftMargin: 30
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20 + guiSettings.footerMargin
            width: 60
            height: width
            background.opacity: 0.2
            iconColor: "white"
            Material.background: guiSettings.fullScreenColor
            svg: SvgOutline.flipCamera
            accessibleName: qsTr("flip camera")
            onClicked: toggleCamera(true)
        }

        Label {
            id: zoomFactor
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: photoButton.top
            anchors.bottomMargin: 10
            padding: 5
            background: Rectangle {
                radius: guiSettings.radius
                opacity: 0.2
                color: guiSettings.fullScreenColor
            }
            color: "white"
            text: `${Math.round(camera.zoomFactor * 10) / 10}x`
            visible: false

            Timer {
                id: hideTimer
                interval: 1000
                onTriggered: zoomFactor.visible = false
            }

            function show() {
                hideTimer.stop()
                zoomFactor.visible = true
            }

            function hide() {
                hideTimer.start()
            }
        }

        SwipeHandler {
            onSwipeDown: toggleCamera(true)
            onSwipeUp: toggleCamera(false)
        }

        ZoomHandler {
            onScaleChanged: (delta) => {
                camera.zoomFactor *= delta
                zoomFactor.show()
            }

            onReleased: zoomFactor.hide()
        }
    }

    Item {
        anchors.fill: videoOutput

        // Add perspective to rotation
        transform: [
            Scale {
                readonly property int angle: Math.abs(videoRotation.angle)

                origin.x: videoOutput.width / 2
                origin.y: videoOutput.height / 2
                xScale: 1 - (angle <= 90 ? angle : (360 - angle)) / 360
                yScale: 1 - (angle <= 90 ? angle : (360 - angle)) / 360
            }
        ]

        MultiEffect {
            id: blur
            source: videoOutput
            anchors.fill: parent
            blurEnabled: true
            blur: 1.0
            blurMax: 64
            autoPaddingEnabled: false
            visible: false

            transform: Rotation {
                id: videoRotation
                origin.x: videoOutput.width / 2
                origin.y: videoOutput.height / 2
                axis { x: 1; y: 0; z: 0 }
                angle: 0
            }

            function activate(active) {
                visible = active
                videoOutput.visible = !active
            }
        }
    }

    Rectangle {
        id: shutter
        anchors.fill: parent
        color: guiSettings.fullScreenColor
        visible: false

        SoundEffect {
            id: shutterSound
            audioDevice: mediaDevices.defaultAudioOutput
            source: "/sounds/camera-shutter-click.wav"
            volume: 1.0
            loops: 1
        }

        Timer {
            id: shutterTimer
            interval: 200
            onTriggered: shutter.visible = false
        }

        function go() {
            shutterSound.play()
            shutter.visible = true
            shutterTimer.start()
        }
    }

    NumberAnimation {
        property var doneCb

        id: rotationAnimation
        target: videoRotation
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

    SvgButton {
        x: guiSettings.leftMargin + 10
        y: guiSettings.headerMargin
        iconColor: "white"
        Material.background: guiSettings.fullScreenColor
        background.opacity: 0.2
        svg: SvgOutline.arrowBack
        accessibleName: qsTr("go back")
        onClicked: page.closed()
    }

    SvgButton {
        anchors.right: parent.right
        anchors.rightMargin: guiSettings.rightMargin + 10
        anchors.top: parent.top
        anchors.topMargin: guiSettings.headerMargin
        iconColor: "white"
        Material.background: guiSettings.fullScreenColor
        background.opacity: 0.2
        svg: getFlashSvg()
        accessibleName: getFlashSpeech()
        onClicked: toggleFlash()
        visible: camera.hasFlash

        function getFlashSvg() {
            switch (camera.flashMode) {
            case Camera.FlashOff:
                return SvgOutline.flashOff
            case Camera.FlashOn:
                return SvgOutline.flashOn
            case Camera.FlashAuto:
                return SvgOutline.flashAuto
            }

            console.warn("Unknown flash mode:", camera.flashMode)
            return Camera.FlashOff
        }

        function getFlashSpeech() {
            switch (camera.flashMode) {
            case Camera.FlashOff:
                return qsTr("flash off")
            case Camera.FlashOn:
                return qsTr("flash on")
            case Camera.FlashAuto:
                return qsTr("auto flash")
            }

            console.warn("Unknown flash mode:", camera.flashMode)
            return qsTr("unknown flash mode")
        }

        function toggleFlash() {
            switch (camera.flashMode) {
            case Camera.FlashOff:
                if (camera.isFlashModeSupported(Camera.FlashOn))
                    camera.flashMode = Camera.FlashOn
                break
            case Camera.FlashOn:
                if (camera.isFlashModeSupported(Camera.FlashAuto))
                    camera.flashMode = Camera.FlashAuto
                break
            case Camera.FlashAuto:
                if (camera.isFlashModeSupported(Camera.FlashOff))
                    camera.flashMode = Camera.FlashOff
                break
            }

            console.warn("Unknown flash mode:", camera.flashMode)
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
        const fileName = cameraUtils.getCaptureFileName()

        if (!fileName) {
            skywalker.showStatusMessage(qsTr("Cannot save file"), QEnums.STATUS_LEVEL_ERROR)
            return
        }

        console.debug("Capture to:", fileName)
        imageCapture.captureToFile(fileName)
        //shutter.go()
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
        console.debug("Camera zoom:", camera.zoomFactor, "min:", camera.minimumZoomFactor, "max:", camera.maximumZoomFactor)
        console.debug("Camera focus mode:", camera.focusMode)
        console.debug("Camera flas mode:", camera.flashMode)
    }
}
