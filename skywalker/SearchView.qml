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

    signal closed

    id: page
    clip: true

    Accessible.role: Accessible.Pane

    header: SearchHeader {
        placeHolderText: isPostSearch ? qsTr("Search posts") : qsTr("Search users")
        onBack: page.closed()

        onSearchTextChanged: (text) => {
            page.isTyping = true

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

        SvgButton {
            id: searchModeToggle
            anchors.verticalCenter: searchModeBar.verticalCenter
            width: height
            height: 35
            radius: 3
            imageMargin: 5
            svg: page.isPostSearch ? svgOutline.chat : svgOutline.user
            onClicked: page.isPostSearch = !page.isPostSearch

            Accessible.role: Accessible.Button
            Accessible.name: getSpeech()
            Accessible.onPressAction: clicked()

            function getSpeech() {
                if (page.isPostSearch)
                    return qsTr("Search mode is posts. Click to change to users.")
                else
                    return qsTr("Search mode is users. Click to change to posts.")
            }
        }

        AccessibleText {
            id: searchModeText
            width: parent.width - searchModeToggle.width - (blockHashtagButton.visible ? blockHashtagButton.width : 0)
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
            visible: isHashtagSearch
            onClicked: muteWord(page.getSearchText())

            Accessible.role: Accessible.Button
            Accessible.name: qsTr(`Mute hashtag ${page.getSearchText()}`)
            Accessible.onPressAction: clicked()
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
        currentIndex: page.isPostSearch ? 0 : 1
        visible: !page.isTyping

        ListView {
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
        page.header.forceFocus()
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
    }
}
