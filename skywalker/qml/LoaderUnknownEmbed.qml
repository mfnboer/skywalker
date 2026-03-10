import QtQuick

Loader {
    required property bool postHasUnknownEmbed
    required property string postUnknownEmbedType
    property bool postVisible: true

    id: loaderUnknownEmbed
    active: postHasUnknownEmbed && postVisible

    sourceComponent: UnknownEmbedView {
        width: loaderUnknownEmbed.width
        unknownEmbedType: postUnknownEmbedType
    }
}
