import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker
import atproto.lib

SkyPage {
    required property var skywalker
    property var userSettings: skywalker.getUserSettings()
    readonly property string userDid: skywalker.getUserDid()
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
        placeHolderText: isPostSearch ? qsTr("Search posts") : qsTr("Search users")
        onBack: page.closed()

        onSearchTextChanged: (text) => {
            page.isTyping = true
            currentText = text

            if (text.length > 0) {
                if (UnicodeFonts.isHashtag(text)) {
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
                page.header.forceFocus()
            }
        }

        onKeyRelease: (event) => {
            if (event.key === Qt.Key_Return) {
                searchUtils.search(getDisplayText())
            }
        }

        onSearch: (text) => { searchUtils.search(text) }
    }

    // Place footer explicitly on the bottom instead of using Page.footer
    // This way the content goes all the way to the bottom underneath the footer.
    SkyFooter {
        id: pageFooter
        width: parent.width
        anchors.bottom: parent.bottom
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
        Material.background: guiSettings.backgroundColor
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

        function setLatestPosts() {
            currentIndex = tabLatestPosts.TabBar.index
        }
    }

    SvgButton {
        id: moreButton
        anchors.right: parent.right
        anchors.rightMargin: 5
        imageMargin: 0
        y: (searchBar.height - height) / 2
        width: height
        height: 30
        iconColor: guiSettings.textColor
        Material.background: "transparent"
        svg: SvgOutline.menu
        accessibleName: qsTr("hashtag options")
        visible: isHashtagSearch

        onClicked: moreMenu.open()

        Menu {
            property bool isMuted: false
            property bool isPinned: false

            id: moreMenu
            modal: true

            onAboutToShow: {
                root.enablePopupShield(true)
                isMuted = skywalker.mutedWords.containsEntry(page.getSearchText())
                isPinned = skywalker.favoriteFeeds.isPinnedSearch(page.getSearchText())
            }

            onAboutToHide: root.enablePopupShield(false)

            CloseMenuItem {
                text: qsTr("<b>Hashtags</b>")
                Accessible.name: qsTr("close hashtag options menu")
            }

            AccessibleMenuItem {
                text: qsTr("Focus hashtag")
                onTriggered: focusHashtag(page.getSearchText())
                MenuItemSvg { svg: SvgOutline.hashtag }
            }

            AccessibleMenuItem {
                text: moreMenu.isMuted ? qsTr("Unmute hashtag") : qsTr("Mute hashtag")
                onTriggered: {
                    if (moreMenu.isMuted)
                        unmuteWord(page.getSearchText())
                    else
                        muteWord(page.getSearchText())
                }

                MenuItemSvg { svg: moreMenu.isMuted ? SvgOutline.unmute : SvgOutline.mute }
            }

            AccessibleMenuItem {
                text: moreMenu.isPinned ? qsTr("Remove favorite") : qsTr("Add favorite")
                onTriggered: {
                    const view = searchUtils.createSearchFeed(page.getSearchText(),
                        postAuthorUser, postMentionsUser,
                        postSetSince ? postSince : nullDate,
                        postSetUntil ? postUntil : nullDate,
                        postLanguage)

                    skywalker.favoriteFeeds.pinSearch(view, !moreMenu.isPinned)
                    skywalker.saveFavoriteFeeds()
                }

                MenuItemSvg {
                    svg: moreMenu.isPinned ? SvgFilled.star : SvgOutline.star
                    color: moreMenu.isPinned ? guiSettings.favoriteColor : guiSettings.textColor
                }
            }
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
        Material.background: guiSettings.backgroundColor
        leftPadding: page.margin
        rightPadding: page.margin
        topPadding: 5
        bottomPadding: 5
        visible: page.isPostSearch

        SkySvg {
            id: restrictionIcon
            y: height
            width: height
            height: 20
            color: guiSettings.linkColor
            svg: SvgOutline.noReplyRestrictions
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
            width: parent.width - implicitHeight - 2 * page.margin
            anchors.verticalCenter: searchModeBar.verticalCenter
            leftPadding: 5
            rightPadding: 5
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            color: guiSettings.linkColor
            text: page.getSearchPostScopeText()

            MouseArea {
                anchors.fill: parent
                onClicked: page.changeSearchPostScope()
                enabled: page.isPostSearch
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
        anchors.bottom: pageFooter.top
        model: searchUtils.authorTypeaheadList
        visible: page.isTyping && !page.isHashtagSearch

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
        visible: page.isTyping && page.isHashtagSearch

        onHashtagClicked: (tag) => {
            const fullTag = `#${tag}`
            page.header.setSearchText(fullTag) // qmllint disable missing-property
            searchUtils.search(fullTag)
        }
    }

    StackLayout {
        id: searchStack
        anchors.top: searchModeSeparator.bottom
        anchors.bottom: pageFooter.top
        width: parent.width
        currentIndex: currentText ?
            searchBar.currentIndex :
            ((page.header.hasFocus() || recentSearchesView.keepFocus || (!userSettings.showTrendingTopics && !userSettings.showSuggestedUsers)) ?
                recentSearchesView.StackLayout.index :
                suggestionsView.StackLayout.index)
        visible: !page.isTyping || !currentText

        onCurrentIndexChanged: {
            if (currentIndex === recentSearchesView.StackLayout.index)
                recentSearchesRepeater.model = searchUtils.getLastSearches()
        }

        SkyListView {
            id: postsViewTop
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height
            model: searchUtils.getSearchPostFeedModel(SearchSortOrder.TOP)
            clip: true

            delegate: PostFeedViewDelegate {
                width: postsViewTop.width
            }

            StackLayout.onIsCurrentItemChanged: {
                if (!StackLayout.isCurrentItem)
                    cover()
            }

            FlickableRefresher {
                scrollToTopButtonMargin: pageFooter.height
                inProgress: searchUtils.searchPostsTopInProgress
                topOvershootFun:  () => searchUtils.scopedRefreshSearchPosts(SearchSortOrder.TOP)
                bottomOvershootFun: () => searchUtils.scopedNextPageSearchPosts(SearchSortOrder.TOP)
                topText: qsTr("Pull down to refresh")
            }

            EmptyListIndication {
                svg: SvgOutline.noPosts
                text: qsTr("No posts found")
                list: postsViewTop
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: searchUtils.searchPostsTopInProgress
            }
        }

        SkyListView {
            id: postsViewLatest
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height
            model: searchUtils.getSearchPostFeedModel(SearchSortOrder.LATEST)
            clip: true

            delegate: PostFeedViewDelegate {
                width: postsViewLatest.width
            }

            StackLayout.onIsCurrentItemChanged: {
                if (!StackLayout.isCurrentItem)
                    cover()
            }

            FlickableRefresher {
                scrollToTopButtonMargin: pageFooter.height
                inProgress: searchUtils.searchPostsLatestInProgress
                topOvershootFun:  () => searchUtils.scopedRefreshSearchPosts(SearchSortOrder.LATEST)
                bottomOvershootFun: () => searchUtils.scopedNextPageSearchPosts(SearchSortOrder.LATEST)
                topText: qsTr("Pull down to refresh")
            }

            EmptyListIndication {
                svg: SvgOutline.noPosts
                text: qsTr("No posts found")
                list: postsViewLatest
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: searchUtils.searchPostsLatestInProgress
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

            FlickableRefresher {
                scrollToTopButtonMargin: pageFooter.height
                inProgress: searchUtils.searchActorsInProgress
                bottomOvershootFun: () => searchUtils.getNextPageSearchActors(header.getDisplayText())
            }

            EmptyListIndication {
                svg: SvgOutline.noUsers
                text: qsTr("No users found")
                list: usersView
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: searchUtils.searchActorsInProgress
            }
        }

        Flickable {
            id: suggestionsView
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height
            clip: true
            contentWidth: page.width
            contentHeight: suggestedUsersView.y + suggestedUsersView.height
            flickableDirection: Flickable.VerticalFlick
            boundsBehavior: Flickable.StopAtBounds
            interactive: trendingTopicsColumn.visible && contentY < trendingTopicsColumn.y + trendingTopicsColumn.height

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
                        height: Math.max(topicIcon.height, topicTitle.height)
                        color: "transparent"


                        SkySvg {
                            id: topicIcon
                            x: page.margin
                            width: 34
                            height: width
                            color: guiSettings.textColor
                            svg: SvgOutline.trending
                        }

                        SkyCleanedTextLine {
                            id: topicTitle
                            anchors.left: topicIcon.right
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.leftMargin: guiSettings.threadColumnWidth - topicIcon.width
                            anchors.rightMargin: page.margin
                            elide: Text.ElideRight
                            color: guiSettings.linkColor
                            plainText: modelData.topic
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: root.openLink(modelData.link)
                        }
                    }
                }
            }

            SkyListView {
                id: suggestedUsersView
                anchors.top: trendingTopicsColumn.visible ? trendingTopicsColumn.bottom : parent.top
                width: suggestionsView.width
                height: visible ? suggestionsView.height : 0
                model: searchUtils.getSearchSuggestedUsersModel()
                clip: true
                interactive: !suggestionsView.interactive
                visible: userSettings.showSuggestedUsers

                onVerticalOvershootChanged: {
                    if (interactive && trendingTopicsColumn.visible)
                        suggestionsView.contentY = y + verticalOvershoot
                }

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
                            guiSettings.notice(page, qsTr("You can enable trending topics again in settings."))
                            userSettings.showSuggestedUsers = false
                        }
                    }
                }

                delegate: AuthorViewDelegate {
                    width: suggestedUsersView.width
                    onFollow: (profile) => { graphUtils.follow(profile) }
                    onUnfollow: (did, uri) => { graphUtils.unfollow(did, uri) }
                }

                FlickableRefresher {
                    scrollToTopButtonMargin: pageFooter.height
                    inProgress: searchUtils.searchSuggestedActorsInProgress
                    bottomOvershootFun: () => searchUtils.getNextPageSuggestedActors()
                }

                EmptyListIndication {
                    y: header.height
                    svg: SvgOutline.noUsers
                    text: qsTr("No suggestions")
                    list: suggestedUsersView
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: searchUtils.searchSuggestedActorsInProgress
                }
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
                height: count > 0 ? 90 : 0
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
                    height: parent.height
                    spacing: 10

                    Avatar {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 60
                        author: modelData

                        onClicked: {
                            page.skywalker.getDetailedProfile(modelData.did)

                            // This call changes the model!
                            searchUtils.addLastSearchedProfile(modelData)
                        }

                        SvgButton {
                            y: -topInset
                            x: parent.width - width + rightInset
                            width: 28
                            height: width
                            imageMargin: 8
                            iconColor: guiSettings.textColor
                            Material.background: guiSettings.backgroundColor
                            flat: true
                            svg: SvgOutline.close
                            accessibleName: qsTr(`remove ${modelData.name}`)
                            onPressed: {
                                page.header.forceFocus()
                                searchUtils.removeLastSearchedProfile(modelData.did)
                            }
                        }
                    }

                    SkyCleanedTextLine {
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

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                header.setSearchText(recentSearchText.text)
                                searchUtils.search(recentSearchText.text)
                            }
                        }

                        SvgButton {
                            id: removeRecentSearchButton
                            anchors.right: parent.right
                            width: 34
                            height: width
                            iconColor: guiSettings.textColor
                            Material.background: guiSettings.backgroundColor
                            flat: true
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
        overrideAdultVisibility: page.overrideAdultVisibility

        function search(query) {
            page.isTyping = false

            if (query.length > 0) {
                searchUtils.addLastSearch(query)
                scopedSearchPosts(query)
                searchUtils.searchActors(query)
            }
            else {
                currentText = "*"
                scopedSearchPosts("*")
            }
        }

        function scopedSearchPosts(query) {
            if (query.length === 0)
                return

            searchPosts(query, SearchSortOrder.TOP, postAuthorUser, postMentionsUser,
                        postSince, postSetSince, postUntil, postSetUntil, postLanguage)
            searchPosts(query, SearchSortOrder.LATEST, postAuthorUser, postMentionsUser,
                        postSince, postSetSince, postUntil, postSetUntil, postLanguage)
        }

        function scopedNextPageSearchPosts(sortOrder) {
            getNextPageSearchPosts(header.getDisplayText(), sortOrder, postAuthorUser,
                                   postMentionsUser, postSince, postSetSince,
                                   postUntil, postSetUntil, postLanguage)
        }

        function scopedRefreshSearchPosts(sortOrder) {
            searchPosts(header.getDisplayText(), sortOrder, postAuthorUser, postMentionsUser,
                        postSince, postSetSince, postUntil, postSetUntil, postLanguage)
        }

        function suggestUsers() {
            if (!userSettings.showSuggestedUsers)
                return

            if (!postAuthorUser || postAuthorUser === "me")
                getSuggestedActors()
            else
                getSuggestedFollows(postAuthorUser)
        }

        function suggestTrendingTopics() {
            if (!userSettings.showTrendingTopics)
                return

            getTrendingTopics()
        }

        function getSuggestions() {
            suggestUsers()
            suggestTrendingTopics()
        }

        Component.onDestruction: {
            // The destuctor of SearchUtils is called too late by the QML engine
            // Remove models now before the Skywalker object is destroyed.
            searchUtils.removeModels()
        }
    }

    GraphUtils {
        id: graphUtils
        skywalker: page.skywalker // qmllint disable missing-type

        onFollowFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }
        onUnfollowFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }
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

        console.debug("AUTHOR:", authorName, otherAuthorHandle)

        let component = Qt.createComponent("SearchPostScope.qml")
        let scopePage = component.createObject(page, {
                skywalker: page.skywalker,
                authorName: authorName,
                otherAuthorHandle: otherAuthorHandle,
                mentionsName: mentionsName,
                otherMentionsHandle: otherMentionsHandle,
                sinceDate: postSince,
                setSince: postSetSince,
                untilDate: postUntil,
                setUntil: postSetUntil,
                language: postLanguage
        })

        let callback = () => {
            postAuthorUser = scopePage.getAuthorName()
            postMentionsUser = scopePage.getMentionsName()
            postSince = scopePage.sinceDate
            postSetSince = scopePage.setSince
            postUntil = scopePage.untilDate
            postSetUntil = scopePage.setUntil
            postLanguage = scopePage.language
            userSettings.setSearchAdultOverrideVisibility(userDid, scopePage.overrideAdultVisibility)
            overrideAdultVisibility = scopePage.overrideAdultVisibility
            searchUtils.scopedSearchPosts(page.getSearchText())
            scopePage.destroy()
        }

        scopePage.onRejected.connect(callback)
        scopePage.onAccepted.connect(callback)
        scopePage.open()
    }

    function getSearchText() {
        return page.header.getDisplayText() // qmllint disable missing-property
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

    function muteWord(word) {
        skywalker.mutedWords.addEntry(word)
        skywalker.saveMutedWords()
        skywalker.showStatusMessage(qsTr(`Muted ${word}`), QEnums.STATUS_LEVEL_INFO)
    }

    function unmuteWord(word) {
        skywalker.mutedWords.removeEntry(word)
        skywalker.saveMutedWords()
        skywalker.showStatusMessage(qsTr(`Unmuted ${word}`), QEnums.STATUS_LEVEL_INFO)
    }

    function focusHashtag(hashtag) {
        let component = Qt.createComponent("FocusHashtags.qml")
        let focusPage = component.createObject(page)
        focusPage.onClosed.connect(() => { root.popStack() }) // qmllint disable missing-property
        skywalker.focusHashtags.addEntry(hashtag.slice(1)) // strip #-symbol
        root.pushStack(focusPage)
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

    function forceDestroy() {
        searchUtils.clearAllSearchResults()
        usersView.model = null
        postsViewTop.model = null
        postsViewLatest.model = null
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
        postAuthorUser = searchFeed.authorHandle
        postMentionsUser = searchFeed.mentionHandle
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
            postAuthorUser = searchScope
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

        if (userSettings.showSuggestedUsers)
            firstSearch = false
    }

    Component.onCompleted: {
        searchUtils.initLastSearchedProfiles()
    }
}
