import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ComboBox {
    property profile selectedAuthor: currentValue ? currentValue : nullProfile
    property profile nullProfile

    id: authorComboBox
    valueRole: "author"

    onCountChanged: currentIndex = 0

    contentItem: AuthorItemDelegate {
        author: selectedAuthor
    }

    delegate: ItemDelegate {
        required property int index
        required property var modelData

        id: delegate
        width: popup.width
        highlighted: authorComboBox.highlightedIndex === index

        contentItem: AuthorItemDelegate {
            author: modelData.author
        }

        background: Rectangle {
            implicitWidth: delegate.width
            color: delegate.highlighted ? Material.listHighlightColor : "transparent"
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
