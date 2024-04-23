import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var skywalker
    property var timeline
    property bool isTyping: true
    property bool isHashtagSearch: false
    property bool isPostSearch: true
    property string postSearchUser // empty, "me", handle
    property string initialSearch
    property string currentText
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
                searchUtils.getSuggestedActors()
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
    }

    Row {
        id: searchModeBar
        width: parent.width
        leftPadding: page.margin
        rightPadding: page.margin

        SvgButton {
            id: searchModeToggle
            anchors.verticalCenter: searchModeBar.verticalCenter
            width: height
            height: 35
            radius: 3
            imageMargin: 5
            svg: page.isPostSearch ? svgOutline.chat : svgOutline.user
            accessibleName: getSpeech()
            onClicked: page.isPostSearch = !page.isPostSearch

            function getSpeech() {
                if (page.isPostSearch)
                    return qsTr("Search mode is posts. Click to change to users.")
                else
                    return qsTr("Search mode is users. Click to change to posts.")
            }
        }

        AccessibleText {
            id: searchModeText
            width: parent.width - searchModeToggle.width - 2 * page.margin - (blockHashtagButton.visible ? blockHashtagButton.width : 0)
            anchors.verticalCenter: searchModeBar.verticalCenter
            elide: Text.ElideRight
            color: page.isPostSearch ? guiSettings.linkColor : guiSettings.textColor
            text: page.isPostSearch ? qsTr(`Posts from ${page.getSearchPostScopeText()}`) : qsTr("Users")

            MouseArea {
                anchors.fill: parent
                onClicked: page.changeSearchPostScope()
                enabled: page.isPostSearch
            }
        }

        SvgButton {
            id: blockHashtagButton
            anchors.verticalCenter: searchModeBar.verticalCenter
            width: height
            height: 35
            imageMargin: 5
            svg: svgOutline.mute
            accessibleName: qsTr(`Mute hashtag ${page.getSearchText()}`)
            visible: isHashtagSearch
            onClicked: muteWord(page.getSearchText())
        }
    }

    Rectangle {
        id: searchModeSeparator
        anchors.top: searchModeBar.bottom
        width: parent.width
        height: 1
        color: guiSettings.separatorColor
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
        currentIndex: currentText ? (page.isPostSearch ? postsView.index : usersView.index) :
                                    (page.header.hasFocus() ? recentSearchesView.index : suggestedUsersView.index)
        visible: !page.isTyping || !currentText

        onCurrentIndexChanged: {
            if (currentIndex === recentSearchesView.index)
                recentSearchesView.model = searchUtils.getLastSearches()
        }

        ListView {
            readonly property int index: 0

            id: postsView
            width: parent.width
            height: parent.height
            spacing: 0
            clip: true
            model: searchUtils.getSearchPostFeedModel()
            flickDeceleration: guiSettings.flickDeceleration
            ScrollIndicator.vertical: ScrollIndicator {}

            Accessible.role: Accessible.List

            delegate: PostFeedViewDelegate {
                width: postsView.width
            }

            FlickableRefresher {
                inProgress: searchUtils.searchPostsInProgress
                verticalOvershoot: postsView.verticalOvershoot
                bottomOvershootFun: () => searchUtils.scopedNextPageSearchPosts()
                topText: ""
            }

            EmptyListIndication {
                svg: svgOutline.noPosts
                text: qsTr("No posts found")
                list: postsView
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: searchUtils.searchPostsInProgress
            }
        }

        ListView {
            readonly property int index: 1

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
                width: postsView.width
                onFollow: (profile) => { graphUtils.follow(profile) }
                onUnfollow: (did, uri) => { graphUtils.unfollow(did, uri) }
            }

            FlickableRefresher {
                inProgress: searchUtils.searchActorsInProgress
                verticalOvershoot: usersView.verticalOvershoot
                bottomOvershootFun: () => searchUtils.getNextPageSearchActors(header.getDisplayText())
                topText: ""
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
            readonly property int index: 2

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
                width: postsView.width
                onFollow: (profile) => { graphUtils.follow(profile) }
                onUnfollow: (did, uri) => { graphUtils.unfollow(did, uri) }
            }

            FlickableRefresher {
                inProgress: searchUtils.searchSuggestedActorsInProgress
                verticalOvershoot: suggestedUsersView.verticalOvershoot
                bottomOvershootFun: () => searchUtils.getNextPageSuggestedActors()
                topText: ""
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
            readonly property int index: 3

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
                topPadding: 10
                bottomPadding: 10
                horizontalAlignment: Text.AlignHCenter
                font.bold: true
                font.pointSize: guiSettings.scaledFont(9/8)
                text: qsTr("Recent searches")
                visible: recentSearchesView.count > 0
            }

            delegate: Rectangle {
                required property string modelData

                width: parent.width
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

            const searchTerm = getPostSearchTerm(query)
            searchPosts(searchTerm)
        }

        function scopedNextPageSearchPosts() {
            const searchTerm = getPostSearchTerm(header.getDisplayText())
            getNextPageSearchPosts(searchTerm)
        }

        function getPostSearchTerm(query) {
            if (postSearchUser)
                return `from:${postSearchUser} ${query}`
            else
                return query
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
        let userName = ""
        let otherHandle = ""

        if (postSearchUser) {
            if (postSearchUser === "me") {
                userName = "me"
            }
            else {
                userName = "other"
                otherHandle = postSearchUser
            }
        }

        let component = Qt.createComponent("SearchPostScope.qml")
        let scopePage = component.createObject(page, { userName: userName, otherUserHandle: otherHandle })
        scopePage.onRejected.connect(() => scopePage.destroy())
        scopePage.onAccepted.connect(() => {
                postSearchUser = scopePage.getUserName()
                searchUtils.scopedSearchPosts(page.getSearchText())
                scopePage.destroy()
        })
        scopePage.open()
    }

    function getSearchText() {
        return page.header.getDisplayText()
    }

    function getSearchPostScopeText() {
        if (postSearchUser === "me")
            return postSearchUser

        return postSearchUser ? `@${postSearchUser}` : "everyone"
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

    function forceDestroy() {
        searchUtils.clearAllSearchResults();
        usersView.model = null
        postsView.model = null
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

        initialSearch = searchText

        if (searchText || searchScope) {
            isPostSearch = true
            postSearchUser = searchScope
            header.setSearchText(searchText)
            searchUtils.search(searchText)
        }
    }

    Component.onCompleted: {
        if (initialSearch)
            searchUtils.search(initialSearch)
        else
            searchUtils.getSuggestedActors()
    }
}
