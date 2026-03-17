import QtQuick
import skywalker

Loader {
    required property basicprofile postAuthor
    required property list<contentlabel> postContentLabels
    property contentlabel filteredContentLabel
    property bool postVisible: true
    property bool moving: false

    id: loaderContentLabels

    active: postContentLabels.length > 0 && postVisible && !moving

    sourceComponent: ContentLabels {
        parentWidth: loaderContentLabels.width
        alignRight: true
        contentLabels: postContentLabels
        filteredContentLabel: loaderContentLabels.filteredContentLabel
        contentAuthor: postAuthor
    }

    onStatusChanged: {
        if (status == Loader.Ready)
            active = true
    }
}
