import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    required property convoview convo
    readonly property string sideBarTitle: qsTr("Reactions")
    readonly property SvgImage sideBarSvg: SvgOutline.like

    signal closed
    signal deleteEmoji(string emoji)

    id: view

    header: SimpleHeader {
        text: sideBarTitle
        visible: root.isPortrait
        onBack: view.closed()
    }
    headerPositioning: ListView.OverlayHeader

    delegate: ReactionViewDelegate {
        required property reactionview modelData

        width: view.width
        convo: view.convo
        reaction: modelData

        onClicked: {
            guiSettings.askYesNoQuestion(view,
                qsTr(`Do you want to delete your reaction: ${reaction.emoji} ?`),
                () => view.deleteEmoji(reaction.emoji))
        }
    }
}
