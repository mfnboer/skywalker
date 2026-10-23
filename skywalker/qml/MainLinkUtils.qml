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

    onWebLink: (link, containingText, hostPresent, warning) => {
        if ((!containingText || hostPresent) && !warning) {
            openInBrowser(link)
            return
        }

        const linkText = `<a href="l" style="color: ${guiSettings.linkColor}; text-decoration: none">${link}</a>`
        const warnText = warning ? "<br><br>⚠️ " + warning : ""

        guiSettings.noticeOkCancel(rootContent,
            qsTr("This link will open the following website:") + "<br><br>" + linkText + warnText,
            () => openInBrowser(link),
            () => {},
            (l) => openInBrowser(link))
    }

    onPostLink: (atUri) => skywalker.getPostThread(atUri)
    onFeedLink: (atUri) => skywalker.getFeedGenerator(atUri, true)
    onListLink: (atUri) => root.viewListByUri(atUri, true)
    onStarterPackLink: (atUri) => skywalker.getStarterPackView(atUri)
    onAuthorLink: (handle) => skywalker.getDetailedProfile(handle)
}
