import QtQuick
import skywalker

Loader {
    property string userDid
    property var postExternal // externalview (var allows NULL)
    required property int postContentVisibility // QEnums::PostContentVisibility
    required property string postContentWarning
    required property basicprofile postContentLabeler
    property string bodyBackgroundColor: guiSettings.backgroundColor
    property bool highlight: bodyBackgroundColor === guiSettings.postHighLightColor
    property bool postVisible: true

    id: loaderExternal
    active: Boolean(postExternal) && postVisible

    sourceComponent: ExternalView {
        userDid: loaderExternal.userDid
        postExternal: loaderExternal.postExternal
        contentVisibility: postContentVisibility
        contentWarning: postContentWarning
        contentLabeler: postContentLabeler
        highlight: loaderExternal.highlight
        maskColor: bodyBackgroundColor
    }
}
