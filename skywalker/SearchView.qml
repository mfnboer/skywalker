import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker
import atproto

Page {
    required property var skywalker
    property var timeline
    property bool isTyping: true
    property bool isHashtagSearch: false
    property bool isPostSearch: true
    property string postAuthorUser // empty, "me", handle
    property string postMentionsUser // empty, "me", handle
    property date postSince
    property bool postSetSince: false
    property date postUntil
    property bool postSetUntil: false
    property string currentText
    property bool firstSearch: true
    readonly property int margin: 10

    signal closed

    id: page
    clip: true

    Accessible.role: Accessible.Pane

    header: SearchHeader {
        minSearchTextLength: 0
        placeHolderText: isPostSearch ? qsTr("Search posts") : qsTr("Search users")
        onBack: page.closed()

        onSearchTextChanged: (text) => {
            page.isTyping = true
            currentText = text

            if (text.length > 0) {
                if (unicodeFonts.isHashtag(text)) {
                    page.isHashtagSearch = true
                    hashtagTypeaheadSearchTimer.start()
                }
                else {
                    page.isHashtagSearch = false
                    authorTypeaheadSearchTimer.start()
                }
            } else {
                authorTypeaheadSearchTimer.stop()
                hashtagTypeaheadSearchTimer.stop()
                searchUtils.authorTypeaheadList = []
                searchUtils.hashtagTypeaheadList = []
                page.isHashtagSearch = false
                searchUtils.suggestUsers()
            }
        }

        onKeyRelease: (event) => {
            if (event.key === Qt.Key_Return) {
                searchUtils.search(getDisplayText())
            }
        }

        onSearch: (text) => { searchUtils.search(text) }
    }

    footer: SkyFooter {
        timeline: page.timeline
        skywalker: page.skywalker
        searchActive: true
        onHomeClicked: root.viewTimeline()
        onNotificationsClicked: root.viewNotifications()
        onFeedsClicked: root.viewFeedsView()
        onMessagesClicked: root.viewChat()
    }

    TabBar {
        id: searchBar
        width: parent.width
        leftPadding: page.margin
        rightPadding: page.margin

        onCurrentIndexChanged: page.isPostSearch = (currentIndex !== tabUsers.TabBar.index)

        AccessibleTabButton {
            id: tabTopPosts
            text: qsTr("Top")
            width: implicitWidth;
        }
        AccessibleTabButton {
            id: tabLatestPosts
            text: qsTr("Latest")
            width: implicitWidth;
        }
        AccessibleTabButton {
            id: tabUsers
            text: qsTr("Users")
            width: implicitWidth;
        }

        function setTopPosts() {
            currentIndex = tabTopPosts.TabBar.index
        }
    }

    Rectangle {
        id: searchBarSeparator
        anchors.top: searchBar.bottom
        width: parent.width
        height: 1
        color: guiSettings.separatorColor
    }

    Row {
        id: searchModeBar
        anchors.top: searchBarSeparator.bottom
        width: parent.width
        height: page.isPostSearch ? implicitHeight : 0
        leftPadding: page.margin
        rightPadding: page.margin
        topPadding: 5
        bottomPadding: 5
        visible: page.isPostSearch

        SvgImage {
            id: restrictionIcon
            y: height
            width: height
            height: searchModeText.height
            color: guiSettings.linkColor
            svg: svgOutline.noReplyRestrictions
            Accessible.ignored: true

            MouseArea {
                y: -height
                width: parent.width
                height: parent.height
                onClicked: page.changeSearchPostScope()
                enabled: page.isPostSearch
            }
        }
        AccessibleText {
            id: searchModeText
            width: parent.width - implicitHeight - 2 * page.margin - (blockHashtagButton.visible ? implicitHeight : 0)
            anchors.verticalCenter: searchModeBar.verticalCenter
            leftPadding: 5
            rightPadding: 5
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            color: page.isPostSearch ? guiSettings.linkColor : guiSettings.textColor
            text: page.isPostSearch ? page.getSearchPostScopeText() : qsTr("Users")

            MouseArea {
                anchors.fill: parent
                onClicked: page.changeSearchPostScope()
                enabled: page.isPostSearch
            }
        }

        SvgImage {
            id: blockHashtagButton
            y: height
            width: height
            height: searchModeText.height
            color: guiSettings.linkColor
            svg: svgOutline.block
            visible: isHashtagSearch

            MouseArea {
                y: -height
                width: parent.width
                height: parent.height
                onClicked: muteWord(page.getSearchText())
            }
        }
    }

    Rectangle {
        id: searchModeSeparator
        anchors.top: searchModeBar.bottom
        width: parent.width
        height: page.isPostSearch ? 1 : 0
        color: guiSettings.separatorColor
        visible: page.isPostSearch
    }

    SimpleAuthorListView {
        id: typeaheadView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: searchModeSeparator.bottom
        anchors.bottom: parent.bottom
        model: searchUtils.authorTypeaheadList
        visible: page.isTyping && !page.isPostSearch && !page.isHashtagSearch

        onAuthorClicked: (profile) => { page.skywalker.getDetailedProfile(profile.did) }
    }

    HashtagListView {
        id: hastagTypeaheadView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: searchModeSeparator.bottom
        anchors.bottom: parent.bottom
        model: searchUtils.hashtagTypeaheadList
        visible: page.isTyping && page.isPostSearch && page.isHashtagSearch

        onHashtagClicked: (tag) => {
            const fullTag = `#${tag}`
            page.header.setSearchText(fullTag)
            searchUtils.search(fullTag)
        }
    }

    StackLayout {
        id: searchStack
        anchors.top: searchModeSeparator.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: currentText ? searchBar.currentIndex :
                                    ((page.header.hasFocus() || recentSearchesView.keepFocus) ?
                                         recentSearchesView.StackLayout.index :
                                         suggestedUsersView.StackLayout.index)
        visible: !page.isTyping || !currentText

        onCurrentIndexChanged: {
            if (currentIndex === recentSearchesView.StackLayout.index)
                recentSearchesView.model = searchUtils.getLastSearches()
        }

        ListView {
            id: postsViewTop
            width: parent.width
            height: parent.height
            spacing: 0
            clip: true
            model: searchUtils.getSearchPostFeedModel(SearchSortOrder.TOP)
            flickDeceleration: guiSettings.flickDeceleration
            ScrollIndicator.vertical: ScrollIndicator {}

            Accessible.role: Accessible.List

            delegate: PostFeedViewDelegate {
                width: postsViewTop.width
            }

            FlickableRefresher {
                inProgress: searchUtils.searchPostsTopInProgress
                topOvershootFun:  () => searchUtils.scopedRefreshSearchPosts(SearchSortOrder.TOP)
                bottomOvershootFun: () => searchUtils.scopedNextPageSearchPosts(SearchSortOrder.TOP)
                topText: qsTr("Pull down to refresh")
            }

            EmptyListIndication {
                svg: svgOutline.noPosts
                text: qsTr("No posts found")
                list: postsViewTop
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: searchUtils.searchPostsTopInProgress
            }
        }

        ListView {
            id: postsViewLatest
            width: parent.width
            height: parent.height
            spacing: 0
            clip: true
            model: searchUtils.getSearchPostFeedModel(SearchSortOrder.LATEST)
            flickDeceleration: guiSettings.flickDeceleration
            ScrollIndicator.vertical: ScrollIndicator {}

            Accessible.role: Accessible.List

            delegate: PostFeedViewDelegate {
                width: postsViewLatest.width
            }

            FlickableRefresher {
                inProgress: searchUtils.searchPostsLatestInProgress
                topOvershootFun:  () => searchUtils.scopedRefreshSearchPosts(SearchSortOrder.LATEST)
                bottomOvershootFun: () => searchUtils.scopedNextPageSearchPosts(SearchSortOrder.LATEST)
                topText: qsTr("Pull down to refresh")
            }

            EmptyListIndication {
                svg: svgOutline.noPosts
                text: qsTr("No posts found")
                list: postsViewLatest
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: searchUtils.searchPostsLatestInProgress
            }
        }

        ListView {
            id: usersView
            width: parent.width
            height: parent.height
            spacing: 0
            clip: true
            model: searchUtils.getSearchUsersModel()
            flickDeceleration: guiSettings.flickDeceleration
            ScrollIndicator.vertical: ScrollIndicator {}

            Accessible.role: Accessible.List

            delegate: AuthorViewDelegate {
                width: usersView.width
                onFollow: (profile) => { graphUtils.follow(profile) }
                onUnfollow: (did, uri) => { graphUtils.unfollow(did, uri) }
            }

            FlickableRefresher {
                inProgress: searchUtils.searchActorsInProgress
                bottomOvershootFun: () => searchUtils.getNextPageSearchActors(header.getDisplayText())
            }

            EmptyListIndication {
                svg: svgOutline.noUsers
                text: qsTr("No users found")
                list: usersView
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: searchUtils.searchActorsInProgress
            }
        }

        ListView {
            id: suggestedUsersView
            width: parent.width
            height: parent.height
            spacing: 0
            clip: true
            model: searchUtils.getSearchSuggestedUsersModel()
            flickDeceleration: guiSettings.flickDeceleration
            ScrollIndicator.vertical: ScrollIndicator {}

            Accessible.role: Accessible.List

            header: AccessibleText {
                width: parent.width
                topPadding: 10
                bottomPadding: 10
                horizontalAlignment: Text.AlignHCenter
                font.bold: true
                font.pointSize: guiSettings.scaledFont(9/8)
                text: qsTr("Suggestions")
                visible: suggestedUsersView.count > 0
            }

            delegate: AuthorViewDelegate {
                width: suggestedUsersView.width
                onFollow: (profile) => { graphUtils.follow(profile) }
                onUnfollow: (did, uri) => { graphUtils.unfollow(did, uri) }
            }

            FlickableRefresher {
                inProgress: searchUtils.searchSuggestedActorsInProgress
                bottomOvershootFun: () => searchUtils.getNextPageSuggestedActors()
            }

            EmptyListIndication {
                svg: svgOutline.noUsers
                text: qsTr("No suggestions")
                list: suggestedUsersView
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: searchUtils.searchSuggestedActorsInProgress
            }
        }

        ListView {
            property bool keepFocus: false

            id: recentSearchesView
            width: parent.width
            height: parent.height
            spacing: 0
            clip: true
            flickDeceleration: guiSettings.flickDeceleration
            ScrollIndicator.vertical: ScrollIndicator {}

            Accessible.role: Accessible.List

            header: AccessibleText {
                width: parent.width
                padding: 10
                font.bold: true
                font.pointSize: guiSettings.scaledFont(9/8)
                text: qsTr("Recent searches")
                visible: recentSearchesView.count > 0

                SvgButton {
                    anchors.right: parent.right
                    width: height
                    height: parent.height
                    svg: svgOutline.close
                    accessibleName: qsTr("clear recent searches")
                    onPressed: {
                        recentSearchesView.keepFocus = true
                        console.debug("CLICKED")
                        page.header.forceFocus()
                        clearRecentSearches()
                    }
                }
            }

            delegate: Rectangle {
                required property string modelData

                width: recentSearchesView.width
                height: Math.max(recentSearchIcon.height, recentSearchText.height)
                color: "transparent"

                SvgImage {
                    id: recentSearchIcon
                    x: page.margin
                    width: 40
                    height: width
                    color: guiSettings.textColor
                    svg: svgOutline.search
                }

                AccessibleText {
                    id: recentSearchText
                    anchors.left: recentSearchIcon.right
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 10
                    anchors.rightMargin: page.margin
                    elide: Text.ElideRight
                    text: modelData
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        header.setSearchText(recentSearchText.text)
                        searchUtils.search(recentSearchText.text)
                    }
                }
            }

            EmptyListIndication {
                svg: svgOutline.search
                text: qsTr("No recent searches")
                list: recentSearchesView
            }
        }
    }

    Timer {
        id: authorTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            const text = page.getSearchText()

            if (text.length > 0)
                searchUtils.searchAuthorsTypeahead(text)
        }
    }

    Timer {
        id: hashtagTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            const text = page.getSearchText()

            if (text.length > 1)
                searchUtils.searchHashtagsTypeahead(text.slice(1)) // strip #-symbol
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: page.skywalker

        function search(query) {
            page.isTyping = false

            if (query.length > 0) {
                searchUtils.addLastSearch(query)
                scopedSearchPosts(query)
                searchUtils.searchActors(query)
            }
        }

        function scopedSearchPosts(query) {
            if (query.length === 0)
                return

            searchPosts(query, SearchSortOrder.TOP, postAuthorUser, postMentionsUser,
                        postSince, postSetSince, postUntil, postSetUntil)
            searchPosts(query, SearchSortOrder.LATEST, postAuthorUser, postMentionsUser,
                        postSince, postSetSince, postUntil, postSetUntil)
        }

        function scopedNextPageSearchPosts(sortOrder) {
            getNextPageSearchPosts(header.getDisplayText(), sortOrder, postAuthorUser,
                                   postMentionsUser, postSince, postSetSince,
                                   postUntil, postSetUntil)
        }

        function scopedRefreshSearchPosts(sortOrder) {
            searchPosts(header.getDisplayText(), sortOrder, postAuthorUser, postMentionsUser,
                        postSince, postSetSince, postUntil, postSetUntil)
        }

        function suggestUsers() {
            if (!postAuthorUser || postAuthorUser === "me")
                getSuggestedActors()
            else
                getSuggestedFollows(postAuthorUser)
        }

        Component.onDestruction: {
            // The destuctor of SearchUtils is called too late by the QML engine
            // Remove models now before the Skywalker object is destroyed.
            searchUtils.removeModels()
        }
    }

    GraphUtils {
        id: graphUtils
        skywalker: page.skywalker

        onFollowFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }
        onUnfollowFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }
    }

    UnicodeFonts {
        id: unicodeFonts
    }

    GuiSettings {
        id: guiSettings
    }

    function changeSearchPostScope() {
        let authorName = "all"
        let otherAuthorHandle = ""
        let mentionsName = "all"
        let otherMentionsHandle = ""

        if (postAuthorUser) {
            if (postAuthorUser === "me") {
                authorName = "me"
            }
            else {
                authorName = "other"
                otherAuthorHandle = postAuthorUser
            }
        }

        if (postMentionsUser) {
            if (postMentionsUser === "me") {
                mentionsName = "me"
            }
            else {
                mentionsName = "other"
                otherMentionsHandle = postMentionsUser
            }
        }

        let component = Qt.createComponent("SearchPostScope.qml")
        let scopePage = component.createObject(page, {
                authorName: authorName,
                otherAuthorHandle: otherAuthorHandle,
                mentionsName: mentionsName,
                sinceDate: postSince,
                setSince: postSetSince,
                untilDate: postUntil,
                setUntil: postSetUntil,
                otherMentionsHandle: otherMentionsHandle
        })

        let callback = () => {
            postAuthorUser = scopePage.getAuthorName()
            postMentionsUser = scopePage.getMentionsName()
            postSince = scopePage.sinceDate
            postSetSince = scopePage.setSince
            postUntil = scopePage.untilDate
            postSetUntil = scopePage.setUntil
            searchUtils.scopedSearchPosts(page.getSearchText())
            scopePage.destroy()
        }

        scopePage.onRejected.connect(callback)
        scopePage.onAccepted.connect(callback)
        scopePage.open()
    }

    function getSearchText() {
        return page.header.getDisplayText()
    }

    function getSearchPostScopeText() {
        let scopeText = ""

        if (postAuthorUser)
        {
            const authorText = (postAuthorUser === "me") ? postAuthorUser : `@${postAuthorUser}`
            scopeText = qsTr(` from:<b>${authorText}</b>`)
        }

        if (postMentionsUser)
        {
            const mentionsText = (postMentionsUser === "me") ? postMentionsUser : `@${postMentionsUser}`
            scopeText += qsTr(` mentions:<b>${mentionsText}</b>`)
        }

        if (postSetSince)
            scopeText += qsTr(` since:<b>${postSince.toLocaleDateString(Qt.locale(), Locale.ShortFormat)}</b>`)

        if (postSetUntil)
            scopeText += qsTr(` until:<b>${postUntil.toLocaleDateString(Qt.locale(), Locale.ShortFormat)}</b>`)

        if (scopeText)
            return qsTr(`Posts${scopeText}`)

        return "All posts"
    }

    function muteWord(word) {
        guiSettings.askYesNoQuestion(
                    page,
                    qsTr(`Mute <font color="${guiSettings.linkColor}">${word}</font> ?`),
                    () => {
                        skywalker.mutedWords.addEntry(word)
                        skywalker.saveMutedWords()
                        skywalker.showStatusMessage(qsTr(`Muted ${word}`), QEnums.STATUS_LEVEL_INFO)
                    })
    }

    function clearRecentSearches() {
        guiSettings.askYesNoQuestion(
                    page,
                    qsTr("Are you sure to clear your recent searches?"),
                    () => {
                        searchUtils.clearLastSearches()
                        recentSearchesView.model = []
                        recentSearchesView.keepFocus = false
                        page.header.forceFocus()
                    },
                    () =>  {
                        recentSearchesView.keepFocus = false
                        page.header.forceFocus()
                    })
    }

    function forceDestroy() {
        searchUtils.clearAllSearchResults();
        usersView.model = null
        postsViewTop.model = null
        postsViewLatest.model = null
        searchUtils.removeModels()
        destroy()
    }

    function hide() {
        page.header.unfocus()
    }

    function show(searchText = "", searchScope = "") {
        if (searchText)
            page.header.forceFocus()
        else
            page.header.unfocus()

        if (searchText || searchScope) {
            searchBar.setTopPosts()
            postAuthorUser = searchScope
            header.setSearchText(searchText)

            if (searchText)
                searchUtils.search(searchText)
            else
                searchUtils.suggestUsers()
        }
        else if (firstSearch) {
            searchUtils.suggestUsers()
        }

        firstSearch = false
    }
}
