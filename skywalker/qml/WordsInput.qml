import QtQuick
import QtQuick.Controls

TextInput {
        readonly property string cursorWord: getCursorWord()

        id: textInput

        function getWordBoundaries() {
            let startPos = 0

            if (cursorPosition > 0) {
                const i = displayText.lastIndexOf(" ", cursorPosition - 1)

                if (i >= 0)
                    startPos = i + 1
            }

            const j = displayText.indexOf(" ", startPos)
            const endPos = j >= 0 ? j : displayText.length

            return [startPos, endPos]
        }

        function getCursorWord() {
            const [startPos, endPos] = getWordBoundaries()
            return displayText.slice(startPos, endPos)
        }

        function replaceCursorWord(newWord) {
            const [startPos, endPos] = getWordBoundaries()
            text = displayText.slice(0, startPos) + newWord + displayText.slice(endPos)
            cursorPosition = startPos + newWord.length

            if (cursorPosition === text.length) {
                text += " "
                ++cursorPosition
            }
        }
    }
