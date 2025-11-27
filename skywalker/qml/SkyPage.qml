import QtQuick.Controls
import QtQuick.Controls.Material

Page {
    // Called when list gets covered by another page
    signal cover

    Material.background: guiSettings.backgroundColor

    function getHeaderHeight() {
        return (header ? header.height : 0) + guiSettings.headerMargin
    }

    function getFooterHeight() {
        return (footer ? footer.height : 0) + guiSettings.footerMargin
    }
}
