import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    required property convoview convo

    signal closed
    signal deleteEmoji(string emoji)

    id: view

    header: SimpleHeader {
        text: qsTr("Reactions")
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
