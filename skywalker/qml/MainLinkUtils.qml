import QtQuick
import skywalker

LinkUtils {
    id: linkUtils

    function openInBrowser(link) {
        if (Qt.platform.os === "android") {
            const userSettings = skywalker.getUserSettings()

            if (userSettings.inAppBrowserEnabled) {
                openInAppBrowser(link)
                return
            }
        }

        Qt.openUrlExternally(link)
    }

    onWebLink: (link, containingText, hostPresent) => {
        if (!containingText || hostPresent) {
            openInBrowser(link)
            return
        }

        const linkText = `<font color="${guiSettings.linkColor}">${link}</font>`
        guiSettings.noticeOkCancel(rootContent,
            qsTr("This link will open the following website:") + "<br><br>" + linkText,
            () => openInBrowser(link))
    }

    onPostLink: (atUri) => skywalker.getPostThread(atUri)
    onFeedLink: (atUri) => skywalker.getFeedGenerator(atUri, true)
    onListLink: (atUri) => root.viewListByUri(atUri, true)
    onStarterPackLink: (atUri) => skywalker.getStarterPackView(atUri)
    onAuthorLink: (handle) => skywalker.getDetailedProfile(handle)
}
