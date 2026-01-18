import QtQuick
import skywalker

SkyTextInput {
    inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
    validator: RegularExpressionValidator { regularExpression: /[^ ]*/ }
    valid: isValid()

    function getLink() {
        return linkUtils.getLinkWithScheme(displayText)
    }

    function isValid() {
        const link = getLink()
        return !Boolean(link) || linkUtils.isWebLink(link)
    }

    LinkUtils {
        id: linkUtils
        skywalker: root.getSkywalker()
    }
}
