import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SimpleAuthorListView {
    required property string title

    signal closed

    id: page

    header: SimpleHeader {
        text: title
        onBack: page.closed()
    }
    headerPositioning: ListView.OverlayHeader

    footer: DeadFooterMargin {}
    footerPositioning: ListView.OverlayFooter

    onAuthorClicked: (profile) => root.getSkywalker().getDetailedProfile(profile.did)
}
