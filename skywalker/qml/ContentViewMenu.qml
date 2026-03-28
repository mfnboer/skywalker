import QtQuick
import skywalker

SkyMenu {
    property int contentMode: QEnums.CONTENT_MODE_UNSPECIFIED
    property int underlyingContentMode: QEnums.CONTENT_MODE_UNSPECIFIED

    signal viewChanged(int contentMode)

    id: viewMenu

    SkyMenuButton {
        text: qsTr("Post view")
        svg: SvgOutline.chat
        popup: viewMenu
        visible: underlyingContentMode === QEnums.CONTENT_MODE_UNSPECIFIED
        onClicked: viewChanged(QEnums.CONTENT_MODE_UNSPECIFIED)
    }
    SkyMenuButton {
        text: qsTr("Media view")
        svg: SvgOutline.image
        popup: viewMenu
        visible: underlyingContentMode === QEnums.CONTENT_MODE_UNSPECIFIED
        onClicked: viewChanged(QEnums.CONTENT_MODE_MEDIA)
    }
    SkyMenuButton {
        text: qsTr("Media gallery")
        svg: SvgOutline.gallery
        popup: viewMenu
        visible: underlyingContentMode === QEnums.CONTENT_MODE_UNSPECIFIED
        onClicked: viewChanged(QEnums.CONTENT_MODE_MEDIA_TILES)
    }
    SkyMenuButton {
        text: qsTr("Video view")
        svg: SvgOutline.film
        popup: viewMenu
        onClicked: viewChanged(QEnums.CONTENT_MODE_VIDEO)
    }
    SkyMenuButton {
        text: qsTr("Video gallery")
        svg: SvgOutline.videoGallery
        popup: viewMenu
        onClicked: viewChanged(QEnums.CONTENT_MODE_VIDEO_TILES)
    }
}
