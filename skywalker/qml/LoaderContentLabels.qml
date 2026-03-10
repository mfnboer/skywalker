import QtQuick
import skywalker

Loader {
    required property basicprofile postAuthor
    required property list<contentlabel> postContentLabels
    property contentlabel filteredContentLabel
    property bool postVisible: true

    id: loaderContentLabels

    active: postContentLabels.length > 0 && postVisible

    sourceComponent: ContentLabels {
        parentWidth: loaderContentLabels.width
        alignRight: true
        contentLabels: postContentLabels
        filteredContentLabel: loaderContentLabels.filteredContentLabel
        contentAuthor: postAuthor
    }
}
