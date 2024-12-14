import QtQuick
import QtQuick.Controls
import skywalker

SkyPage {
    required property string imgSource
    property bool sourceIsVideo: false
    property alias text: altText.text
    property var skywalker: root.getSkywalker()
    property var userSettings: skywalker.getUserSettings()
    readonly property int margin: 10

    signal altTextChanged(string text)

    id: page
    width: parent.width
    topPadding: 10
    bottomPadding: 10

    header: SimpleButtonHeader {
        title: qsTr("ALT text")
        buttonSvg: SvgOutline.check
        enabled: !altText.maxGraphemeLengthExceeded()
        onButtonClicked: altTextChanged(page.text)
    }

    // Needed for SkyFormattedTextEdit
    footer: Rectangle {
        height: 0
        color: "transparent"
    }

    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: altImage.y + altImage.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        onHeightChanged: altText.ensureVisible(altText.cursorRectangle)

        SkyFormattedTextEdit {
            id: altText
            width: parent.width
            leftPadding: page.margin
            rightPadding: page.margin
            parentPage: page
            parentFlick: flick
            maxLength: 2000
            placeholderText: qsTr("Help users with visual impairments")
        }

        Image {
            id: altImage
            anchors.leftMargin: page.margin
            anchors.left: parent.left
            anchors.topMargin: 10
            anchors.top: altText.bottom
            width: 240
            height: 180
            fillMode: Image.PreserveAspectCrop
            autoTransform: true
            source: !sourceIsVideo ? page.imgSource : ""
            visible: !sourceIsVideo

            SvgButton {
                x: parent.width - width
                width: 34
                height: width
                svg: SvgOutline.moreVert
                accessibleName: qsTr("change script to extract")
                enabled: extractButton.enabled
                onClicked: moreMenu.open()

                Menu {
                    id: moreMenu
                    modal: true

                    CloseMenuItem {
                        text: qsTr("<b>Scripts</b>")
                        Accessible.name: qsTr("close scripts menu")
                    }

                    ScriptRecognitionMenuItem {
                        script: QEnums.SCRIPT_LATIN
                        ButtonGroup.group: scriptButtonGroup
                    }
                    ScriptRecognitionMenuItem {
                        script: QEnums.SCRIPT_CHINESE
                        ButtonGroup.group: scriptButtonGroup
                    }
                    ScriptRecognitionMenuItem {
                        script: QEnums.SCRIPT_DEVANAGARI
                        ButtonGroup.group: scriptButtonGroup
                    }
                    ScriptRecognitionMenuItem {
                        script: QEnums.SCRIPT_JAPANESE
                        ButtonGroup.group: scriptButtonGroup
                    }
                    ScriptRecognitionMenuItem {
                        script: QEnums.SCRIPT_KOREAN
                        ButtonGroup.group: scriptButtonGroup
                    }
                }

                ButtonGroup { id: scriptButtonGroup }
            }

            SkyButton {
                id: extractButton
                x: parent.width - width
                y: parent.height - height
                height: 34
                text: qsTr(`Extract text (${(qEnums.scriptToString(userSettings.scriptRecognition))})`)
                enabled: altText.text.length === 0 && !imageUtils.extractingText && !imageUtils.installing
                onClicked: imageUtils.installModule(userSettings.scriptRecognition)
            }
        }

        VideoThumbnail {
            id: altVideo
            anchors.leftMargin: page.margin
            anchors.left: parent.left
            anchors.topMargin: 10
            anchors.top: altText.bottom
            width: Math.min(height * 1.777, page.width - 2 * page.margin)
            height: 180
            videoSource: sourceIsVideo ? page.imgSource : ""
            visible: sourceIsVideo
        }
    }

    ImageUtils {
        id: imageUtils

        onInstallModuleProgress: (script, progressPercentage) => { // qmllint disable signal-handler-parameters
            skywalker.showStatusMessage(qsTr(`Installing ${(qEnums.scriptToString(script))} recognition: ${progressPercentage}%`), QEnums.STATUS_LEVEL_INFO, 30)
        }

        onInstallModuleOk: (script) => { // qmllint disable signal-handler-parameters
            skywalker.showStatusMessage(qsTr("Extracting text"), QEnums.STATUS_LEVEL_INFO, 30)
            imageUtils.extractText(script, imgSource)
        }

        onInstallModuleFailed: (script, error) => { // qmllint disable signal-handler-parameters
            skywalker.showStatusMessage(qsTr(`Failed to install module for ${(qEnums.scriptToString(script))} text recognition: ${error}`), QEnums.STATUS_LEVEL_INFO)
        }

        onExtractTextOk: (source, extractedText) => {
            if (extractedText) {
                root.clearStatusMessage()
                altText.text = extractedText
            }
            else {
                skywalker.showStatusMessage(qsTr("No text found"), QEnums.STATUS_LEVEL_INFO)
            }
        }

        onExtractTextFailed: (source, error) => {
            skywalker.showStatusMessage(qsTr(`Failed to extract text: ${error}`), QEnums.STATUS_LEVEL_ERROR)
        }
    }

    QEnums {
        id: qEnums
    }

    VirtualKeyboardPageResizer {
        id: virtualKeyboardPageResizer
    }

    Component.onCompleted: {
        // Save the full page height now. Later when the Android keyboard pops up,
        // the page height sometimes changes by itself, but not always...
        virtualKeyboardPageResizer.fullPageHeight = parent.height

        altText.forceActiveFocus()
    }
}
