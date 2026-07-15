import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker
import atproto.lib

SkyPage {
    property Skywalker skywalker: root.getSkywalker()
    property var userSettings: skywalker.getUserSettings()
    readonly property string userDid: skywalker.getUserDid()
    property var timeline
    property bool isTyping: false
    property bool isHashtagSearch: false
    property bool isCashtagSearch: false
    property bool isPostSearch: true
    property bool exactPhrase: false
    property int postFilter: SearchOptions.POST_FILTER_ALL
    property int mediaPostFilter: SearchOptions.MEDIA_POST_FILTER_ALL
    property bool following: false
    property list<string> postAuthorUsers: [] // empty or list of "me" and real handles
    property list<string> postMentionUsers: [] // empty or list of "me" and real handles
    property date postSince
    property bool postSetSince: false
    property date postUntil
    property bool postSetUntil: false
    property string postLanguage
    property bool adultContent: false
    property int overrideAdultVisibility: userSettings.getSearchAdultOverrideVisibility(userDid)
    property string currentText
    property bool firstSearch: true
    readonly property int margin: 10
    property date nullDate

    signal closed

    id: page

    Accessible.role: Accessible.Pane

    onCover: {
        postsViewTop.cover()
        postsViewLatest.cover()
    }

    header: SearchHeader {
        minSearchTextLength: 0
        placeHolderText: qsTr("Search posts, users or feeds")
        showBackButton: !root.showSideBar
        onBack: page.closed()

        onSearchTextChanged: (text) => {
            page.isTyping = true
            currentText = text

            if (text.length > 0) {
                if (UnicodeFonts.isHashtag(text)) {
                    page.isHashtagSearch = true
                    page.isCashtagSearch = false
                    hashtagTypeaheadSearchTimer.start()
                } else if (UnicodeFonts.isCashtag(text)) {
                    page.isHashtagSearch = false
                    page.isCashtagSearch = true
                } else {
                    page.isHashtagSearch = false
                    page.isCashtagSearch = false
                    authorTypeaheadSearchTimer.start()
                }
            } else {
                authorTypeaheadSearchTimer.stop()
                hashtagTypeaheadSearchTimer.stop()
                searchUtils.authorTypeaheadList = []
                searchUtils.hashtagTypeaheadList = []
                page.isHashtagSearch = false
                page.isCashtagSearch = false
                page.header.forceFocus()
            }
        }

        onSearch: (text) => { searchUtils.search(text) }
        onCleared: {
            page.isTyping = false
            currentText = ""
            unfocus()
        }
    }

    // Place footer explicitly on the bottom instead of using Page.footer
    // This way the content goes all the way to the bottom underneath the footer.
    SkyFooter {
        id: pageFooter
        width: parent.width
        anchors.bottom: parent.bottom
        timeline: page.timeline
        skywalker: page.skywalker
        searchView: page
        activePage: QEnums.UI_PAGE_SEARCH
        onHomeClicked: root.viewTimeline()
        onSearchClicked: page.header.forceFocus()
        onNotificationsClicked: root.viewNotifications()
        onMessagesClicked: root.viewChat()
        footerVisible: !root.showSideBar
    }

    SimpleAuthorListView {
        id: typeaheadView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: searchModeSeparator.bottom
        anchors.bottom: pageFooter.top
        model: searchUtils.authorTypeaheadList
        visible: page.isTyping && currentText && !page.isHashtagSearch && !page.isCashtagSearch

        onAuthorClicked: (profile) => {
            searchUtils.addLastSearchedProfile(profile)
            page.skywalker.getDetailedProfile(profile.did)
        }
    }

    HashtagListView {
        id: hastagTypeaheadView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: searchModeSeparator.bottom
        anchors.bottom: pageFooter.top
        model: searchUtils.hashtagTypeaheadList
        visible: page.isTyping && currentText && page.isHashtagSearch

        onHashtagClicked: (tag) => {
            const fullTag = `#${tag}`
            page.header.setSearchText(fullTag) // qmllint disable missing-property
            searchUtils.search(fullTag)
        }
    }

    SkyTabBar {
        id: searchBar
        width: parent.width
        Material.background: guiSettings.backgroundColor
        leftPadding: page.margin
        rightPadding: page.margin
        visible: searchStack.visible

        onCurrentIndexChanged: page.isPostSearch = (currentIndex === tabTopPosts.TabBar.index || currentIndex === tabLatestPosts.TabBar.index)

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
        AccessibleTabButton {
            id: tabFeeds
            text: qsTr("Feeds")
            width: implicitWidth;
        }

        function setTopPosts() {
            currentIndex = tabTopPosts.TabBar.index
        }

        function setLatestPosts() {
            currentIndex = tabLatestPosts.TabBar.index
        }
    }

    SvgPlainButton {
        id: moreButton
        anchors.right: parent.right
        anchors.rightMargin: 5
        imageMargin: 0
        y: (searchBar.height - height) / 2
        width: height
        height: 30
        svg: SvgOutline.menu
        accessibleName: qsTr("tag options")
        visible: isHashtagSearch || isCashtagSearch

        onClicked: moreMenu.show(page.getSearchText())

        HashtagContextMenu {
            id: moreMenu
            following: page.following
            postAuthorUsers: page.postAuthorUsers
            postMentionUsers: page.postMentionUsers
            postSince: page.postSince
            postSetSince: page.postSetSince
            postUntil: page.postUntil
            postSetUntil: page.postSetUntil
            postLanguage: page.postLanguage
        }
    }

    Rectangle {
        id: searchBarSeparator
        anchors.top: searchBar.bottom
        width: parent.width
        height: 1
        color: guiSettings.separatorColor
    }

    RowLayout {
        id: searchModeBar
        x: page.margin
        anchors.top: searchBarSeparator.bottom
        anchors.topMargin: 5
        width: parent.width - 2 * page.margin
        height: page.isPostSearch ? implicitHeight : 0
        Material.background: guiSettings.backgroundColor
        visible: page.isPostSearch

        SvgPlainButton {
            id: restrictionIcon
            Layout.preferredWidth: height
            Layout.preferredHeight: 20
            imageMargin: 0
            iconColor: guiSettings.linkColor
            Material.background: guiSettings.backgroundColor
            svg: SvgOutline.noReplyRestrictions
            accessibleName: searchModeText.text
            onClicked: page.changeSearchPostScope()
        }
        AccessibleText {
            id: searchModeText
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            leftPadding: 5
            rightPadding: 5
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            color: guiSettings.linkColor
            textFormat: Text.StyledText
            text: page.getSearchPostScopeText()

            SkyMouseArea {
                anchors.fill: parent
                onClicked: page.changeSearchPostScope()
                enabled: page.isPostSearch
            }
        }
        SvgPlainButton {
            id: clearScopeButton
            Layout.preferredWidth: 20
            Layout.preferredHeight: width
            imageMargin: 0
            Material.background: guiSettings.backgroundColor
            svg: SvgOutline.close
            accessibleName: qsTr("clear search scope")
            visible: postAuthorUsers.length > 0 || postMentionUsers.length > 0 || postSetSince || postSetUntil || postLanguage
            onClicked: page.clearSearchPostScope()
        }
    }

    Rectangle {
        id: searchModeSeparator
        anchors.top: searchModeBar.bottom
        anchors.topMargin: 5
        width: parent.width
        height: page.isPostSearch ? 1 : 0
        color: guiSettings.separatorColor
        visible: page.isPostSearch
    }

    SwipeView {
        id: searchStack
        anchors.top: searchModeSeparator.bottom
        anchors.bottom: pageFooter.top
        width: parent.width
        currentIndex: searchBar.currentIndex
        visible: currentText && !page.isTyping

        onCurrentIndexChanged: {
            searchBar.setCurrentIndex(currentIndex)
        }

        SkyListView {
            id: postsViewTop
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height
            model: searchUtils.getSearchPostFeedModel(SearchSortOrder.TOP, SearchSortOrder.TOP, true)
            preloadNextPageFunc: () => searchUtils.scopedNextPageSearchPosts(SearchSortOrder.TOP)
            clip: true

            delegate: PostFeedViewDelegate {
                width: postsViewTop.width
            }

            SwipeView.onIsCurrentItemChanged: {
                if (SwipeView.isCurrentItem) {
                    if (count === 0)
                        refreshSearch()
                } else {
                    cover()
                }
            }

            FlickableRefresher {
                inProgress: postsViewTop.model && postsViewTop.model.getFeedInProgress
                topOvershootFun:  () => parent.refreshSearch()
                bottomOvershootFun: () => searchUtils.scopedNextPageSearchPosts(SearchSortOrder.TOP)
                topText: qsTr("Pull down to refresh")
            }

            EmptyListIndication {
                svg: SvgOutline.noPosts
                text: qsTr("No posts found")
                list: postsViewTop
                onRetry: parent.refreshSearch()
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: postsViewTop.model && postsViewTop.model.getFeedInProgress
            }

            function refreshSearch() {
                searchUtils?.scopedRefreshSearchPosts(SearchSortOrder.TOP)
            }
        }

        SkyListView {
            id: postsViewLatest
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height
            model: searchUtils.getSearchPostFeedModel(SearchSortOrder.RECENT, SearchSortOrder.RECENT, true)
            preloadNextPageFunc: () => searchUtils.scopedNextPageSearchPosts(SearchSortOrder.RECENT)
            clip: true

            delegate: PostFeedViewDelegate {
                width: postsViewLatest.width
            }

            SwipeView.onIsCurrentItemChanged: {
                if (SwipeView.isCurrentItem) {
                    if (count === 0)
                        refreshSearch()
                } else {
                    cover()
                }
            }

            FlickableRefresher {
                inProgress: postsViewLatest.model && postsViewLatest.model.getFeedInProgress
                topOvershootFun:  () => parent.refreshSearch()
                bottomOvershootFun: () => searchUtils.scopedNextPageSearchPosts(SearchSortOrder.RECENT)
                topText: qsTr("Pull down to refresh")
            }

            EmptyListIndication {
                svg: SvgOutline.noPosts
                text: qsTr("No posts found")
                list: postsViewLatest
                onRetry: parent.refreshSearch()
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: postsViewLatest.model && postsViewLatest.model.getFeedInProgress
            }

            function refreshSearch() {
                searchUtils?.scopedRefreshSearchPosts(SearchSortOrder.RECENT)
            }
        }

        SkyListView {
            id: usersView
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height
            model: searchUtils.getSearchUsersModel()
            clip: true

            Accessible.role: Accessible.List

            delegate: AuthorViewDelegate {
                width: usersView.width
                onFollow: (profile) => graphUtils.follow(profile)
                onUnfollow: (did, uri) => graphUtils.unfollow(did, uri)
                onClicked: (profile) => searchUtils.addLastSearchedProfile(profile)
            }

            SwipeView.onIsCurrentItemChanged: {
                if (SwipeView.isCurrentItem) {
                    if (count === 0)
                        refreshSearch()
                }
            }

            FlickableRefresher {
                inProgress: usersView.model && usersView.model.getFeedInProgress
                bottomOvershootFun: () => searchUtils.getNextPageSearchActors(page.getSearchText())
            }

            EmptyListIndication {
                svg: SvgOutline.noUsers
                text: qsTr("No users found")
                list: usersView
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: usersView.model && usersView.model.getFeedInProgress
            }

            function refreshSearch() {
                searchUtils?.searchActors(page.getSearchText())
            }
        }

        SkyListView {
            id: feedListView
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height
            model: searchUtils.getSearchFeedsModel()
            clip: true

            Accessible.role: Accessible.List

            delegate: GeneratorViewDelegate {
                width: feedListView.width
                onHideFollowing: (feed, hide) => feedUtils.hideFollowing(feed.uri, hide)
                onSyncFeed: (feed, sync) => feedUtils.syncFeed(feed.uri, sync)
            }

            SwipeView.onIsCurrentItemChanged: {
                if (SwipeView.isCurrentItem) {
                    if (count === 0)
                        refreshSearch()
                }
            }

            FlickableRefresher {
                inProgress: feedListView.model && feedListView.model.getFeedInProgress
                bottomOvershootFun: () => searchUtils.getNextPageSearchFeeds(page.getSearchText())
            }

            EmptyListIndication {
                svg: SvgOutline.noPosts
                text: qsTr("No feeds found")
                list: feedListView
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: feedListView.model && feedListView.model.getFeedInProgress
            }

            function refreshSearch() {
                searchUtils?.searchFeeds(page.getSearchText())
            }
        }
    }

    StackLayout {
        anchors.top: searchModeSeparator.bottom
        anchors.bottom: pageFooter.top
        width: parent.width
        currentIndex: (page.header.hasFocus() || recentSearchesView.keepFocus ||
                       (!userSettings.showTrendingTopics && !userSettings.showSuggestedUsers && !userSettings.showSuggestedFeeds && !userSettings.showSuggestedStarterPacks)) ?
                recentSearchesView.StackLayout.index :
                suggestionsView.StackLayout.index
        visible: !currentText

        onCurrentIndexChanged: {
            if (currentIndex === recentSearchesView.StackLayout.index)
                recentSearchesRepeater.model = searchUtils.getLastSearches()
        }

        Flickable {
            property int lastMovementY: 0

            id: suggestionsView
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height
            clip: true
            contentWidth: page.width
            contentHeight: suggestedStarterPacksView.y + suggestedStarterPacksView.height
            flickableDirection: Flickable.VerticalFlick
            boundsBehavior: Flickable.StopAtBounds
            interactive: true

            onVisibleChanged: {
                // Restore last scroll position when view becomes visible again
                if (visible) {
                    // Delay by few ms to let the complete view render first
                    restorePositionTimer.run(lastMovementY)
                }
            }

            onMovementEnded: {
                lastMovementY = contentY
            }

            function resetPosition() {
                lastMovementY = 0
                contentY = 0
            }

            Timer {
                property int restoreY: 0

                id: restorePositionTimer
                interval: 10
                onTriggered: {
                    if (restoreY <= suggestionsView.contentHeight)
                        suggestionsView.contentY = restoreY
                    else
                        suggestionsView.lastMovementY = suggestionsView.contentY
                }

                function run(posY) {
                    restoreY = posY
                    start()
                }
            }

            AccessibleText {
                id: trendingTopicsText
                width: parent.width
                padding: 10
                font.bold: true
                font.pointSize: guiSettings.scaledFont(9/8)
                text: qsTr("Trending topics")
                visible: userSettings.showTrendingTopics && topicRepeater.count > 0

                SvgButton {
                    anchors.right: parent.right
                    width: height
                    height: parent.height
                    svg: SvgOutline.close
                    accessibleName: qsTr("disable trending topics")
                    onPressed: {
                        guiSettings.notice(page, qsTr("You can enable trending topics again in settings."))
                        userSettings.showTrendingTopics = false
                    }
                }
            }

            Column {
                id: trendingTopicsColumn
                anchors.top: trendingTopicsText.bottom
                width: parent.width
                visible: trendingTopicsText.visible

                Repeater {
                    id: topicRepeater
                    model: searchUtils.trendingTopicsListModel

                    Rectangle {
                        required property var modelData

                        width: parent.width
                        height: Math.max(topicIcon.height, separator.y + separator.height)
                        color: "transparent"

                        SkySvg {
                            id: topicIcon
                            x: page.margin
                            y: height + 10
                            width: 34
                            height: width
                            color: guiSettings.textColor
                            svg: modelData.topic.contentMode === QEnums.CONTENT_MODE_VIDEO ? SvgOutline.film : SvgOutline.trending
                        }

                        RowLayout {
                            id: topicTitle
                            y: 10
                            anchors.left: topicIcon.right
                            anchors.right: parent.right
                            anchors.leftMargin: guiSettings.threadColumnWidth - topicIcon.width
                            anchors.rightMargin: page.margin

                            SkyCleanedTextLine {
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                                font.bold: true
                                plainText: modelData.topic.topic + (modelData.topic.status === QEnums.TREND_STATUS_HOT ? " 🔥" : "")
                            }

                            DurationLabel {
                                Layout.alignment: Qt.AlignVCenter
                                durationSeconds: modelData.topicAgeSeconds
                                visible: modelData.topicAgeSeconds > 0
                            }
                        }

                        RowLayout {
                            id: topicDetails
                            anchors.left: topicTitle.left
                            anchors.right: topicTitle.right
                            anchors.top: topicTitle.bottom

                            SkyCleanedTextLine {
                                topPadding: 5
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                                font.pointSize: guiSettings.scaledFont(7/8)
                                font.italic: true
                                plainText: modelData.topic.category
                            }

                            AccessibleText {
                                topPadding: 5
                                font.pointSize: guiSettings.scaledFont(7/8)
                                color: Material.color(Material.Grey)
                                text: qsTr(`${modelData.topic.postCount} posts`)
                                visible: modelData.topic.postCount > 0
                            }
                        }

                        Rectangle {
                            id: separator
                            anchors.top: topicDetails.bottom
                            anchors.topMargin: 10
                            width: parent.width
                            height: 1
                            color: guiSettings.separatorColor
                        }

                        SkyMouseArea {
                            anchors.fill: parent
                            onClicked: root.openLink(modelData.topic.link)
                        }
                    }
                }
            }

            SkyListView {
                id: suggestedFeedsView
                anchors.top: trendingTopicsColumn.visible ? trendingTopicsColumn.bottom : parent.top
                width: suggestionsView.width
                height: visible ? contentHeight : 0
                model: searchUtils.getSuggestedFeedsModel()
                clip: true
                interactive: false
                visible: userSettings.showSuggestedFeeds

                Accessible.role: Accessible.List

                header: AccessibleText {
                    width: parent.width
                    padding: 10
                    font.bold: true
                    font.pointSize: guiSettings.scaledFont(9/8)
                    text: qsTr("Suggested feeds")

                    SvgButton {
                        anchors.right: parent.right
                        width: height
                        height: parent.height
                        svg: SvgOutline.close
                        accessibleName: qsTr("disable suggested feeds")
                        onPressed: {
                            guiSettings.notice(page, qsTr("You can enable suggested feeds again in settings."))
                            userSettings.showSuggestedFeeds = false
                        }
                    }
                }

                delegate: GeneratorViewDelegate {
                    width: suggestedUsersView.width
                    endOfFeed: false
                    onHideFollowing: (feed, hide) => feedUtils.hideFollowing(feed.uri, hide)
                    onSyncFeed: (feed, sync) => feedUtils.syncFeed(feed.uri, sync)
                }

                EmptyListIndication {
                    y: header.height
                    svg: SvgOutline.noPosts
                    text: qsTr("No suggestions")
                    list: suggestedFeedsView
                }
            }

            SkyListView {
                id: suggestedUsersView
                anchors.top: suggestedFeedsView.bottom
                width: suggestionsView.width
                height: visible ? contentHeight : 0
                model: searchUtils.getSearchSuggestedUsersModel()
                clip: true
                interactive: false
                visible: userSettings.showSuggestedUsers

                Accessible.role: Accessible.List

                header: AccessibleText {
                    width: parent.width
                    padding: 10
                    font.bold: true
                    font.pointSize: guiSettings.scaledFont(9/8)
                    text: qsTr("Suggested accounts")

                    SvgButton {
                        anchors.right: parent.right
                        width: height
                        height: parent.height
                        svg: SvgOutline.close
                        accessibleName: qsTr("disable suggested accounts")
                        onPressed: {
                            guiSettings.notice(page, qsTr("You can enable suggested accounts again in settings."))
                            userSettings.showSuggestedUsers = false
                        }
                    }
                }

                delegate: AuthorViewDelegate {
                    width: suggestedUsersView.width
                    endOfList: false
                    onFollow: (profile) => { graphUtils.follow(profile) }
                    onUnfollow: (did, uri) => { graphUtils.unfollow(did, uri) }
                }

                EmptyListIndication {
                    y: header.height
                    svg: SvgOutline.noUsers
                    text: qsTr("No suggestions")
                    list: suggestedUsersView
                }
            }

            SkyListView {
                id: suggestedStarterPacksView
                anchors.top: suggestedUsersView.bottom
                width: suggestionsView.width
                height: visible ? contentHeight : 0
                model: searchUtils.getSuggestedStarterPacksModel()
                clip: true
                interactive: false
                visible: userSettings.showSuggestedStarterPacks

                Accessible.role: Accessible.List

                header: AccessibleText {
                    width: parent.width
                    padding: 10
                    font.bold: true
                    font.pointSize: guiSettings.scaledFont(9/8)
                    text: qsTr("Suggested starter packs")

                    SvgButton {
                        anchors.right: parent.right
                        width: height
                        height: parent.height
                        svg: SvgOutline.close
                        accessibleName: qsTr("disable suggested starter packs")
                        onPressed: {
                            guiSettings.notice(page, qsTr("You can enable suggested starter packs again in settings."))
                            userSettings.showSuggestedStarterPacks = false
                        }
                    }
                }

                delegate: StarterPackViewDelegate {
                    width: suggestedUsersView.width
                }

                EmptyListIndication {
                    y: header.height
                    svg: SvgOutline.noLists
                    text: qsTr("No suggestions")
                    list: suggestedStarterPacksView
                }
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: suggestedFeedsView.model && suggestedFeedsView.model.getFeedInProgress
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: suggestedUsersView.model && suggestedUsersView.model.getFeedInProgress
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: suggestedStarterPacksView.model && suggestedStarterPacksView.model.getFeedInProgress
            }
        }

        Flickable {
            property bool keepFocus: false

            id: recentSearchesView
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height
            clip: true
            contentWidth: page.width
            contentHeight: recentSearchesColumn.y + recentSearchesColumn.height
            flickableDirection: Flickable.VerticalFlick
            boundsBehavior: Flickable.StopAtBounds

            AccessibleText {
                id: recentSearchesText
                width: parent.width
                padding: 10
                font.bold: true
                font.pointSize: guiSettings.scaledFont(9/8)
                text: qsTr("Recent searches")
                visible: recentProfileSearchList.count + recentSearchesRepeater.count > 0

                SvgButton {
                    anchors.right: parent.right
                    width: height
                    height: parent.height
                    svg: SvgOutline.close
                    accessibleName: qsTr("clear recent searches")
                    onPressed: {
                        recentSearchesView.keepFocus = true
                        page.header.forceFocus()
                        clearRecentSearches()
                    }
                }
            }

            ListView {
                id: recentProfileSearchList
                x: page.margin
                anchors.top: recentSearchesText.bottom
                width: parent.width - page.margin * 2
                height: count > 0 ? 70 + 20 * guiSettings.fontScaleFactor : 0
                model: searchUtils.lastSearchedProfiles
                orientation: ListView.Horizontal
                flickDeceleration: guiSettings.flickDeceleration
                boundsBehavior: Flickable.StopAtBounds
                maximumFlickVelocity: guiSettings.maxFlickVelocity
                pixelAligned: guiSettings.flickPixelAligned
                ScrollIndicator.horizontal: ScrollIndicator {}
                clip: true
                visible: count > 0

                delegate: Column {
                    required property basicprofile modelData

                    width: 80
                    height: parent ? parent.height : 0
                    spacing: 10

                    Avatar {
                        id: profileAvatar
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 60
                        author: modelData

                        onClicked: {
                            page.skywalker.getDetailedProfile(modelData.did)

                            // This call changes the model!
                            searchUtils.addLastSearchedProfile(modelData)
                        }

                        SvgPlainButton {
                            y: -topInset
                            x: parent.width - width + rightInset
                            width: 28
                            height: width
                            imageMargin: 8
                            Material.background: guiSettings.backgroundColor
                            svg: SvgOutline.close
                            accessibleName: qsTr(`remove ${modelData.name}`)
                            onPressed: {
                                page.header.forceFocus()
                                searchUtils.removeLastSearchedProfile(modelData.did)
                            }
                        }
                    }

                    SkyCleanedTextLine {
                        id: profileName
                        width: parent.width
                        anchors.horizontalCenter: parent.horizontalCenter
                        elide: Text.ElideRight
                        font.pointSize: guiSettings.scaledFont(6/8)
                        horizontalAlignment: Text.AlignHCenter
                        plainText: modelData.name
                    }
                }
            }

            Column {
                id: recentSearchesColumn
                anchors.top: recentProfileSearchList.bottom
                width: parent.width
                visible: recentSearchesRepeater.count > 0

                Repeater {
                    id: recentSearchesRepeater

                    delegate: Rectangle {
                        required property string modelData

                        width: parent.width
                        height: Math.max(recentSearchIcon.height, recentSearchText.height)
                        color: "transparent"

                        SkySvg {
                            id: recentSearchIcon
                            x: page.margin
                            width: 34
                            height: width
                            color: guiSettings.textColor
                            svg: SvgOutline.search
                        }

                        AccessibleText {
                            id: recentSearchText
                            anchors.left: recentSearchIcon.right
                            anchors.right: removeRecentSearchButton.left
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.leftMargin: 10
                            anchors.rightMargin: page.margin
                            elide: Text.ElideRight
                            text: modelData
                        }

                        SkyMouseArea {
                            anchors.fill: parent
                            onClicked: {
                                header.setSearchText(recentSearchText.text)
                                searchUtils.search(recentSearchText.text)
                            }
                        }

                        SvgPlainButton {
                            id: removeRecentSearchButton
                            anchors.right: parent.right
                            width: 34
                            height: width
                            Material.background: guiSettings.backgroundColor
                            svg: SvgOutline.close
                            accessibleName: qsTr(`remove ${recentSearchText.text}`)
                            onPressed: {
                                page.header.forceFocus()
                                searchUtils.removeLastSearch(recentSearchText.text)
                                recentSearchesRepeater.model = searchUtils.getLastSearches()
                            }
                        }
                    }
                }
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
        skywalker: page.skywalker // qmllint disable missing-type
        overrideAdultVisibility: page.overrideAdultVisibility // qmllint disable missing-type

        function search(query) {
            page.isTyping = false
            page.resetSearch()

            if (query.length > 0) {
                if (searchStack.currentItem)
                    searchStack.currentItem.refreshSearch()

                searchUtils.addLastSearch(query)
            }
            else {
                currentText = "*"
                // scopedSearchPosts("*")

                if (searchStack.currentItem)
                    searchStack.currentItem.refreshSearch()
            }
        }

        function scopedSearchPosts(query) {
            if (query.length === 0)
                return

            scopedRefreshSearchPosts(SearchSortOrder.TOP)
            scopedRefreshSearchPosts(SearchSortOrder.RECENT)
        }

        function scopedNextPageSearchPosts(sortOrder) {
            getNextPageSearchPosts(currentText, getSearchOptions(sortOrder))
        }

        function scopedRefreshSearchPosts(sortOrder) {
            if (currentText === "*" && postAuthorUsers.length === 0 && postMentionUsers.length === 0)
                return

            searchPosts(currentText, getSearchOptions(sortOrder))
        }

        function getSearchOptions(sortOrder) {
            let searchOptions = searchUtils.makeSearchOptions()
            searchOptions.sortOrder = sortOrder
            searchOptions.exactPhrase = exactPhrase
            searchOptions.postFilter = postFilter
            searchOptions.mediaPostFilter = mediaPostFilter
            searchOptions.following = following
            searchOptions.authors = postAuthorUsers
            searchOptions.mentions = postMentionUsers
            searchOptions.since = postSince
            searchOptions.isSetSince = postSetSince
            searchOptions.until = postUntil
            searchOptions.isSetUntil = postSetUntil
            searchOptions.language = postLanguage
            return searchOptions
        }

        function suggestUsers() {
            if (!userSettings.showSuggestedUsers)
                return

            if (postAuthorUsers.length === 0 || postAuthorUsers === ["me"])
                getSuggestedActors()
            else
                getSuggestedFollows(postAuthorUsers[0])
        }

        function suggestTrendingTopics() {
            if (!userSettings.showTrendingTopics)
                return

            getTrendingTopics()
        }

        function suggestFeeds() {
            if (!userSettings.showSuggestedFeeds)
                return

            getSuggestedFeeds()
        }

        function suggestStarterPacks() {
            if (!userSettings.showSuggestedStarterPacks)
                return

            getSuggestedStarterPacks()
        }

        function getSuggestions() {
            suggestUsers()
            suggestTrendingTopics()
            suggestFeeds()
            suggestStarterPacks()
        }

        Component.onDestruction: {
            // The destuctor of SearchUtils is called too late by the QML engine
            // Remove models now before the Skywalker object is destroyed.
            searchUtils.removeModels()
        }
    }

    FeedUtils {
        id: feedUtils
        skywalker: page.skywalker // qmllint disable missing-type
    }

    GraphUtils {
        id: graphUtils
        skywalker: page.skywalker // qmllint disable missing-type

        onFollowFailed: (error) => { skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR) }
        onUnfollowFailed: (error) => { skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR) }
    }

    function clearSearchPostScope() {
        postAuthorUsers = []
        postMentionUsers = []
        postSetSince = false
        postSince = nullDate
        postSetUntil = false
        postUntil = nullDate
        postLanguage = ""
    }

    function changeSearchPostScope() {
        let authorName = "all"
        let otherAuthorHandles = ""
        let mentionsName = "all"
        let otherMentionsHandles = ""

        if (postAuthorUsers.length > 0) {
            if (postAuthorUsers === ["me"]) {
                authorName = "me"
            }
            else {
                authorName = "other"
                otherAuthorHandles = postAuthorUsers.join(" ")
            }
        } else if (following) {
            authorName = "following"
        }

        if (postMentionUsers.length > 0) {
            if (postMentionUsers === ["me"]) {
                mentionsName = "me"
            }
            else {
                mentionsName = "other"
                otherMentionsHandles = postMentionUsers.join(" ")
            }
        }

        console.debug("AUTHOR:", authorName, otherAuthorHandles)

        let component = guiSettings.createComponent("SearchPostScope.qml")
        let scopePage = component.createObject(page, {
                skywalker: page.skywalker,
                exactPhrase: exactPhrase,
                postFilter: postFilter,
                mediaPostFilter: mediaPostFilter,
                authorName: authorName,
                otherAuthorHandles: otherAuthorHandles,
                mentionsName: mentionsName,
                otherMentionsHandles: otherMentionsHandles,
                sinceDate: postSince,
                setSince: postSetSince,
                untilDate: postUntil,
                setUntil: postSetUntil,
                language: postLanguage
        })

        let callback = () => {
            exactPhrase = scopePage.exactPhrase
            postFilter = scopePage.postFilter
            mediaPostFilter = scopePage.mediaPostFilter
            following = scopePage.getFollowing()
            postAuthorUsers = scopePage.getAuthorNames()
            postMentionUsers = scopePage.getMentionNames()
            postSince = scopePage.sinceDate
            postSetSince = scopePage.setSince
            postUntil = scopePage.untilDate
            postSetUntil = scopePage.setUntil
            postLanguage = scopePage.language
            userSettings.setSearchAdultOverrideVisibility(userDid, scopePage.overrideAdultVisibility)
            overrideAdultVisibility = scopePage.overrideAdultVisibility
            searchUtils.scopedSearchPosts(page.getSearchText())
            scopePage.close()
        }

        scopePage.onRejected.connect(callback)
        scopePage.onAccepted.connect(callback)
        scopePage.open()
    }

    function getSearchText() {
        return page.header.displayText
    }

    function getAuthorListText(authors) {
        let authorText = ""

        for (const author of authors) {
            if (authorText)
                authorText += " "

            authorText += author === "me" ? author : `@${author}`
        }

        return authorText
    }

    function getSearchPostScopeText() {
        let scopeText = ""

        if (following)
            scopeText += qsTr(" from:<b>users you follow</b>")

        if (postAuthorUsers.length > 0)
        {
            const authorText = getAuthorListText(postAuthorUsers)
            scopeText += qsTr(` from:<b>${authorText}</b>`)
        }

        if (postMentionUsers.length > 0)
        {
            const mentionsText = getAuthorListText(postMentionUsers)
            scopeText += qsTr(` mentions:<b>${mentionsText}</b>`)
        }

        if (postSetSince)
            scopeText += qsTr(` since:<b>${postSince.toLocaleDateString(Qt.locale(), Locale.ShortFormat)}</b>`)

        if (postSetUntil)
            scopeText += qsTr(` until:<b>${postUntil.toLocaleDateString(Qt.locale(), Locale.ShortFormat)}</b>`)

        if (postLanguage)
            scopeText += qsTr(` language:<b>${postLanguage}</b>`)

        let adult = ""

        if (adultContent)
            adult = `, adult: ${getAdultVisibility()}`

        if (scopeText)
            return qsTr(`Posts${scopeText}${adult}`)

        return qsTr(`All posts${adult}`)
    }

    function getAdultVisibility() {
        switch (overrideAdultVisibility) {
        case QEnums.CONTENT_VISIBILITY_SHOW:
            return qsTr("on")
        case QEnums.CONTENT_VISIBILITY_WARN_MEDIA:
            return qsTr("warn")
        case QEnums.CONTENT_VISIBILITY_HIDE_MEDIA:
            return qsTr("hide")
        default:
            return qsTr("unknown")
        }
    }

    function clearRecentSearches() {
        guiSettings.askYesNoQuestion(
                    page,
                    qsTr("Are you sure to clear your recent searches?"),
                    () => {
                        searchUtils.clearLastSearches()
                        recentSearchesRepeater.model = []
                        recentSearchesView.keepFocus = false
                        page.header.forceFocus()
                    },
                    () =>  {
                        recentSearchesView.keepFocus = false
                        page.header.forceFocus()
                    })
    }

    function resetSearch() {
        feedListView.model.clear()
        usersView.model.clear()
        postsViewTop.model.clear()
        postsViewLatest.model.clear()
    }

    function getHeaderHeight() {
        return searchStack.y + header.height + guiSettings.headerMargin
    }

    function getFooterHeight() {
        return pageFooter.height + guiSettings.footerMargin
    }

    function forceDestroy() {
        searchUtils.clearAllSearchResults()
        searchUtils.removeModels()
        destroy()
    }

    function hide() {
        page.header.unfocus() // qmllint disable missing-property
    }

    function showSearchFeed(searchFeed) {
        adultContent = skywalker.getContentFilter().getAdultContent()
        page.header.forceFocus()
        searchBar.setLatestPosts()
        following = searchFeed.following
        postAuthorUsers = searchFeed.authorHandles
        postMentionUsers = searchFeed.mentionHandles
        postSince = searchFeed.since
        postSetSince = !isNaN(searchFeed.since.getTime())
        postUntil = searchFeed.until
        postSetUntil = !isNaN(searchFeed.until.getTime())
        postLanguage = searchFeed.language
        header.setSearchText(searchFeed.searchQuery)
        searchUtils.search(searchFeed.searchQuery)
    }

    function show(searchText = "", searchScope = "") {
        adultContent = skywalker.getContentFilter().getAdultContent()

        if (searchText)
            page.header.forceFocus()
        else
            page.header.unfocus()

        if (searchText || searchScope) {
            searchBar.setTopPosts()
            postAuthorUsers = searchScope ? [searchScope] : []
            header.setSearchText(searchText)

            if (searchText)
                searchUtils.search(searchText)
            else
                searchUtils.getSuggestions()
        }
        else if (firstSearch) {
            searchUtils.getSuggestions()
        }
        else {
            searchUtils.suggestTrendingTopics()
        }

        if (userSettings.showSuggestedUsers || userSettings.showSuggestedFeeds || userSettings.showSuggestedStarterPacks)
            firstSearch = false

        suggestionsView.resetPosition()
    }

    Component.onCompleted: {
        userSettings.onShowSuggestedUsersChanged.connect(() => { if (userSettings.showSuggestedUsers) firstSearch = true })
        userSettings.onShowSuggestedFeedsChanged.connect(() => { if (userSettings.showSuggestedFeeds) firstSearch = true })
        userSettings.onShowSuggestedStarterPacksChanged.connect(() => { if (userSettings.showSuggestedStarterPacks) firstSearch = true })
        searchUtils.initLastSearchedProfiles()
        header.unfocus()
    }
}
