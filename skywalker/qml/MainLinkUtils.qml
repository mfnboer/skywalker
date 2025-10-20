import QtQuick
import skywalker

LinkUtils {
    id: linkUtils

    onWebLink: (link, containingText, hostPresent) => {
        if (!containingText || hostPresent) {
            Qt.openUrlExternally(link)
            return
        }

        const linkText = `<font color="${guiSettings.linkColor}">${link}</font>`
        guiSettings.noticeOkCancel(rootContent,
            qsTr("This link will open the following website:") + "<br><br>" + linkText,
            () => Qt.openUrlExternally(link))
    }
    onPostLink: (atUri) => skywalker.getPostThread(atUri)
    onFeedLink: (atUri) => skywalker.getFeedGenerator(atUri, true)
    onListLink: (atUri) => root.viewListByUri(atUri, true)
    onStarterPackLink: (atUri) => skywalker.getStarterPackView(atUri)
    onAuthorLink: (handle) => skywalker.getDetailedProfile(handle)
}
