import QtQuick
import skywalker

Item {
    property string text: ""
    property list<string> images: []
    property list<string> altTexts: []
    property basicprofile quoteAuthor
    property string quoteUri: ""
    property string quoteCid: ""
    property string quoteText: ""
    property date quoteDateTime: new Date()
    property generatorview quoteFeed
    property listview quoteList
    property var gif: null
    property var card: null
}
