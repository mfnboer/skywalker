// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "facet_utils.h"
#include "skywalker.h"
#include "unicode_fonts.h"
#include <atproto/lib/at_uri.h>

namespace Skywalker {

inline bool isCursorInMatch(int cursor, const ATProto::RichTextMaster::ParsedMatch& match)
{
    return cursor >= match.mStartIndex && cursor <= match.mEndIndex;
}

FacetUtils::FacetUtils(QObject* parent) :
    WrappedSkywalker(parent),
    Presence()
{
    mFacetHighlighter.setEmbeddedLinks(&mEmbeddedLinks);
    connect(this, &FacetUtils::embeddedLinksChanged, this, [this]{ mFacetHighlighter.rehighlight(); });
}

void FacetUtils::setHighlightDocument(QQuickTextDocument* doc, const QString& highlightColor,
                                     const QString& errorColor, int maxLength, const QString& lengthExceededColor)
{
    mFacetHighlighter.setDocument(doc->textDocument());
    mFacetHighlighter.setHighlightColor(highlightColor);
    mFacetHighlighter.setErrorColor(errorColor);
    mFacetHighlighter.setMaxLength(maxLength, lengthExceededColor);
}

void FacetUtils::setHighLightMaxLength(int maxLength)
{
    mFacetHighlighter.setMaxLength(maxLength);
}

void FacetUtils::extractMentionsAndLinks(const QString& text, const QString& preeditText,
                                        int cursor)
{
    if (cursor < 0)
        cursor = text.size();

    const QString fullText = text.sliced(0, cursor) + preeditText + text.sliced(cursor);
    const auto facets = ATProto::RichTextMaster::parseFacets(fullText);

    int preeditCursor = cursor + preeditText.length();
    bool editMentionFound = false;
    bool editTagFound = false;
    bool webLinkFound = false;
    bool postLinkFound = false;
    bool feedLinkFound = false;
    bool listLinkFound = false;
    QStringList mentions;
    WebLink::List webLinks;
    int cursorInWebLink = -1;
    mLinkShorteningReduction = 0;
    QString textWithoutLinks = "";
    int textIndex = 0;

    for (const auto& facet : facets)
    {
        if (facetOverlapsWithEmbeddedLink(facet))
            continue;

        switch (facet.mType)
        {
        case ATProto::RichTextMaster::ParsedMatch::Type::LINK:
        {
            const auto atUri = ATProto::ATUri::fromHttpsPostUri(facet.mMatch);

            if (atUri.isValid())
            {
                qDebug() << "Valid post link:" << facet.mMatch;

                if (!postLinkFound)
                {
                    setFirstPostLink(facet, cursor);
                    postLinkFound = true;
                }
            }
            else
            {
                const auto atFeedUri = ATProto::ATUri::fromHttpsFeedUri(facet.mMatch);

                if (atFeedUri.isValid())
                {
                    qDebug() << "Valid feed link:" << facet.mMatch;

                    if (!feedLinkFound)
                    {
                        setFirstFeedLink(facet, cursor);
                        feedLinkFound = true;
                    }
                }
                else
                {
                    const auto atListUri = ATProto::ATUri::fromHttpsListUri(facet.mMatch);

                    if (atListUri.isValid())
                    {
                        qDebug() << "Valid list link:" << facet.mMatch;

                        if (!listLinkFound)
                        {
                            setFirstListLink(facet, cursor);
                            listLinkFound = true;
                        }
                    }
                    else
                    {
                        qDebug() << "Web link:" << facet.mMatch;
                        webLinks.push_back(WebLink(facet.mMatch, facet.mStartIndex, facet.mEndIndex));

                        if (isCursorInMatch(cursor, facet))
                            cursorInWebLink = webLinks.size() - 1;

                        if (!webLinkFound)
                        {
                            qDebug() << "First web link:" << facet.mMatch;
                            setFirstWebLink(facet, cursor);
                            webLinkFound = true;
                        }
                    }
                }
            }

            const auto shortLink = ATProto::RichTextMaster::shortenWebLink(facet.mMatch);
            const int reduction = UnicodeFonts::graphemeLength(facet.mMatch) - UnicodeFonts::graphemeLength(shortLink);
            qDebug() << "SHORT:" << shortLink << "reduction:" << reduction;
            mLinkShorteningReduction += reduction;
            textWithoutLinks += fullText.sliced(textIndex, facet.mStartIndex - textIndex);
            textIndex = facet.mEndIndex;
            break;
        }
        case ATProto::RichTextMaster::ParsedMatch::Type::PARTIAL_MENTION:
        case ATProto::RichTextMaster::ParsedMatch::Type::MENTION:
            if (facet.mStartIndex < preeditCursor && preeditCursor <= facet.mEndIndex)
            {
                mEditMentionIndex = facet.mStartIndex + 1;
                setEditMention(facet.mMatch.sliced(1)); // strip @-symbol
                editMentionFound = true;
            }

            mentions.push_back(facet.mMatch.sliced(1));
            textWithoutLinks += fullText.sliced(textIndex, facet.mStartIndex - textIndex);
            textIndex = facet.mEndIndex;
            break;
        case ATProto::RichTextMaster::ParsedMatch::Type::TAG:
            if (facet.mStartIndex < preeditCursor && preeditCursor <= facet.mEndIndex)
            {
                mEditTagIndex = facet.mStartIndex + 1;
                setEditTag(facet.mMatch.sliced(1)); // strip #-symbol
                editTagFound = true;
            }
            break;
        case ATProto::RichTextMaster::ParsedMatch::Type::UNKNOWN:
            break;
        }
    }

    setMentions(mentions);
    setWebLinks(webLinks);
    setCursorInWebLink(cursorInWebLink);
    textWithoutLinks += fullText.sliced(textIndex);
    setTextWithoutLinks(textWithoutLinks);

    if (!editMentionFound)
        setEditMention({});

    if (!editTagFound)
        setEditTag({});

    if (!webLinkFound)
        setFirstWebLink({});

    if (!postLinkFound)
        setFirstPostLink({});

    if (!feedLinkFound)
        setFirstFeedLink({});

    if (!listLinkFound)
        setFirstListLink({});
}

bool FacetUtils::facetOverlapsWithEmbeddedLink(const ATProto::RichTextMaster::ParsedMatch& facet) const
{
    for (const auto& link : mEmbeddedLinks)
    {
        if (facet.mStartIndex < link.getEndIndex() && facet.mEndIndex > link.getStartIndex())
        {
            qDebug() << "Overlap, facet:" << facet.mMatch << facet.mStartIndex << facet.mEndIndex << "link:" << link.getName() << link.getStartIndex() << link.getEndIndex();
            return true;
        }
    }

    return false;
}

bool FacetUtils::checkMisleadingEmbeddedLinks() const
{
    Q_ASSERT(mSkywalker);

    for (const auto& link : mEmbeddedLinks)
    {
        if (link.hasMisleadingName())
        {
            mSkywalker->showStatusMessage(link.getMisleadingNameError(), QEnums::STATUS_LEVEL_ERROR);
            return false;
        }

        if (link.isTouchingOtherLink())
        {
            mSkywalker->showStatusMessage(tr("Seperate embedded links by white space"), QEnums::STATUS_LEVEL_ERROR);
            return false;
        }
    }

    return true;
}

WebLink FacetUtils::makeWebLink(const QString& name, const QString& link, int startIndex, int endIndex) const
{
    return WebLink(link, startIndex, endIndex, name);
}

void FacetUtils::addEmbeddedLink(const WebLink& link)
{
    qDebug() << "Add embedded link:" << link.getName() << "link:" << link.getLink() << "start:" << link.getStartIndex() << "end:" << link.getEndIndex();
    Q_ASSERT(link.isValidEmbeddedLink());

    if (!link.isValidEmbeddedLink())
        return;

    if (mEmbeddedLinks.contains(link))
    {
        qWarning() << "Link aready added:" << link.getName() << "link:" << link.getLink() << "start:" << link.getStartIndex() << "end:" << link.getEndIndex();
        return;
    }

    mEmbeddedLinks.push_back(link);
    signalEmbeddedLinksUpdated();
}

void FacetUtils::updatedEmbeddedLink(int linkIndex, const WebLink& link)
{
    qDebug() << "Update embedded link:" << linkIndex << "name:" << link.getName() << "link:" << link.getLink() << "start:" << link.getStartIndex() << "end:" << link.getEndIndex();
    Q_ASSERT(linkIndex >= 0);
    Q_ASSERT(linkIndex < mEmbeddedLinks.size());

    if (linkIndex < 0 || linkIndex >= mEmbeddedLinks.size())
    {
        qWarning() << "Link index out of range:" << linkIndex << "size:" << mEmbeddedLinks.size() << "name:" << link.getName() << "link:" << link.getLink() << "start:" << link.getStartIndex() << "end:" << link.getEndIndex();
        return;
    }

    if (mEmbeddedLinks[linkIndex] == link)
    {
        qDebug() << "Link not changed:" << linkIndex;
        return;
    }

    mEmbeddedLinks[linkIndex] = link;
    signalEmbeddedLinksUpdated();
}

void FacetUtils::removeEmbeddedLink(int linkIndex)
{
    Q_ASSERT(linkIndex >= 0);
    Q_ASSERT(linkIndex < mEmbeddedLinks.size());

    if (linkIndex < 0 || linkIndex >= mEmbeddedLinks.size())
    {
        qWarning() << "Link index out of range:" << linkIndex << "size:" << mEmbeddedLinks.size();
        return;
    }

    qDebug() << "Delete embedded link:" << linkIndex << mEmbeddedLinks[linkIndex].getLink();

    if (mCursorInEmbeddedLink == linkIndex)
        setCursorInEmbeddedLink(-1);

    mEmbeddedLinks.remove(linkIndex);
    signalEmbeddedLinksUpdated();
}

void FacetUtils::removeEmbeddedLinkNoSignal(int linkIndex)
{
    Q_ASSERT(linkIndex >= 0);
    Q_ASSERT(linkIndex < mEmbeddedLinks.size());

    qDebug() << "Delete embedded link:" << linkIndex << mEmbeddedLinks[linkIndex].getLink();

    if (mCursorInEmbeddedLink == linkIndex)
        mCursorInEmbeddedLink = -1;

    mEmbeddedLinks.remove(linkIndex);
}

int FacetUtils::getLinkIndexForCursor(const WebLink::List& links, int cursor)
{
    for (int i = 0; i < (int)links.size(); ++i)
    {
        const auto& link = links[i];

        if (cursor >= link.getStartIndex() && cursor <= link.getEndIndex())
            return i;
    }

    return -1;
}

void FacetUtils::updateCursor(int cursor)
{
    setCursorInWebLink(getLinkIndexForCursor(mWebLinks, cursor));
    setCursorInEmbeddedLink(getLinkIndexForCursor(mEmbeddedLinks, cursor));
}

void FacetUtils::updateText(const QString& prevText, const QString& text)
{
    if (mEmbeddedLinks.empty())
        return;

    const TextDiffer::Result diff = TextDiffer::diff(prevText, text);
    qDebug() << "Updated diff:" << (int)diff.mType << "prev:" << prevText << "text:" << text;

    switch (diff.mType)
    {
    case TextDiffType::NONE:
        break;
    case TextDiffType::INSERTED:
        updateEmbeddedLinksInsertedText(diff, text);
        break;
    case TextDiffType::DELETED:
        updateEmbeddedLinksDeletedText(diff);
        break;
    case TextDiffType::REPLACED:
        updateEmbeddedLinksUpdatedText(diff, text);
        break;
    }
}

void FacetUtils::signalEmbeddedLinksUpdated()
{
    verifyEmbeddedLinks();
    emit embeddedLinksChanged();
}

void FacetUtils::verifyEmbeddedLinks()
{
    std::unordered_set<int> endIndexes;

    for (auto& link : mEmbeddedLinks)
        endIndexes.insert(link.getEndIndex());

    for (auto& link : mEmbeddedLinks)
    {
        const bool touching = endIndexes.contains(link.getStartIndex());
        link.setTouchingOtherLink(touching);
    }
}

void FacetUtils::updateEmbeddedLinksInsertedText(const TextDiffer::Result& diff, const QString& text)
{
    qDebug() << "Inserted:" << diff.mNewStartIndex << diff.mNewEndIndex;
    const int diffLength = diff.mNewEndIndex - diff.mNewStartIndex + 1;
    bool updated = false;

    for (int i = 0; i < mEmbeddedLinks.size(); )
    {
        auto& link = mEmbeddedLinks[i];
        qDebug() << "Link:" << link.getName() << "start:" << link.getStartIndex() << "end:" << link.getEndIndex();

        if (link.getStartIndex() >= diff.mNewStartIndex)
        {
            // Text inserted before link
            qDebug() << "Move link:" << diffLength;
            link.addToIndexes(diffLength);
            updated = true;
        }
        else if (link.getEndIndex() > diff.mNewStartIndex)
        {
            // Text inserted inside link
            link.setEndIndex(link.getEndIndex() + diffLength);
            const int nameLength = link.getEndIndex() - link.getStartIndex();
            QString name = text.mid(link.getStartIndex(), nameLength);
            const int newlineIndex = name.indexOf('\n');

            if (newlineIndex >= 0)
            {
                qDebug() << "Remove index from name:" << name;
                name = name.sliced(0, newlineIndex).trimmed();
                link.setEndIndex(link.getStartIndex() + name.length());
            }

            if (name.isEmpty())
            {
                qDebug() << "Remove link due to newline:" << link.getLink();
                removeEmbeddedLinkNoSignal(i);
                updated = true;
                continue;
            }

            qDebug() << "Name:" << name;
            link.setName(name);
            updated = true;
        }

        ++i;
    }

    if (updated)
    {
        signalEmbeddedLinksUpdated();
        emit cursorInEmbeddedLinkChanged();
    }
}

void FacetUtils::updateEmbeddedLinksDeletedText(const TextDiffer::Result& diff)
{
    qDebug() << "Deleted:" << diff.mOldStartIndex << diff.mOldEndIndex;
    const int diffLength = diff.mOldEndIndex - diff.mOldStartIndex + 1;
    bool updated = false;

    for (int i = 0; i < mEmbeddedLinks.size(); )
    {
        auto& link = mEmbeddedLinks[i];
        qDebug() << "Link:" << link.getName() << "start:" << link.getStartIndex() << "end:" << link.getEndIndex();

        if (link.getStartIndex() > diff.mOldEndIndex)
        {
            // Text removed before link
            qDebug() << "Move link:" << -diffLength;
            link.addToIndexes(-diffLength);
            updated = true;
        }
        else if (link.getStartIndex() >= diff.mOldStartIndex && link.getEndIndex() <= diff.mOldEndIndex + 1)
        {
            // Deleted text contains full link
            qDebug() << "Remove link:" << link.getName();
            removeEmbeddedLinkNoSignal(i);
            updated = true;
            continue;
        }
        else if (link.getStartIndex() >= diff.mOldStartIndex && link.getEndIndex() > diff.mOldEndIndex + 1)
        {
            // Deleted text overlaps with prefix of link
            int move = link.getStartIndex() - diff.mOldStartIndex;
            int pos = diff.mOldEndIndex - link.getStartIndex() + 1;
            const QString& linkName = link.getName();

            while (pos < linkName.size() && linkName.at(pos).isSpace())
            {
                ++pos;
                --move;
            }

            if (pos >= linkName.size())
            {
                // No name left
                qDebug() << "Remove link:" << link.getName();
                removeEmbeddedLinkNoSignal(i);
                updated = true;
                continue;
            }

            const QString name = link.getName().sliced(pos);
            qDebug() << "Name:" << name << "move:" << -move;
            link.setName(name);
            link.setStartIndex(link.getStartIndex() - move);
            link.setEndIndex(link.getStartIndex() + name.length());
            updated = true;
        }
        else if (link.getStartIndex() < diff.mOldStartIndex && link.getEndIndex() > diff.mOldEndIndex + 1)
        {
            // Deleted text is inside link
            const QString& oldName = link.getName();
            const int prefixLength = diff.mOldStartIndex - link.getStartIndex();
            const QString nameBefore = oldName.sliced(0, prefixLength);
            const int suffixLength = link.getEndIndex() - (diff.mOldEndIndex + 1);
            const QString nameAfter = oldName.sliced(oldName.length() - suffixLength);
            const QString name = nameBefore + nameAfter;
            qDebug() << "Name:" << name;
            link.setName(name);
            link.setEndIndex(link.getStartIndex() + name.length());
            updated = true;
        }
        else if (link.getStartIndex() < diff.mOldStartIndex && link.getEndIndex() > diff.mOldStartIndex)
        {
            // Deleted text overlaps with suffix of link
            int length = diff.mOldStartIndex - link.getStartIndex();
            const QString& linkName = link.getName();

            while (length > 0 && linkName.at(length - 1).isSpace())
                --length;

            if (length <= 0)
            {
                // No name left
                qDebug() << "Remove link:" << link.getName();
                removeEmbeddedLinkNoSignal(i);
                updated = true;
                continue;
            }

            const QString name = link.getName().sliced(0, length);
            qDebug() << "Name:" << name;
            link.setName(name);
            link.setEndIndex(link.getStartIndex() + length);
            updated = true;
        }

        ++i;
    }

    if (updated)
    {
        signalEmbeddedLinksUpdated();
        emit cursorInEmbeddedLinkChanged();
    }
}

void FacetUtils::updateEmbeddedLinksUpdatedText(const TextDiffer::Result& diff, const QString& text)
{
    qDebug() << "Replaced, old:" << diff.mOldStartIndex << diff.mOldEndIndex << "new:" << diff.mNewStartIndex << diff.mNewEndIndex;
    updateEmbeddedLinksDeletedText(diff);
    updateEmbeddedLinksInsertedText(diff, text);
}

QString FacetUtils::applyFontToLastTypedChars(const QString& text,const QString& preeditText,
                                             int cursor, int numChars, QEnums::FontType font)
{
    if (font == QEnums::FONT_NORMAL)
        return {};

    if (!mEditMention.isEmpty())
    {
        qDebug() << "Editing a mention";
        return {};
    }

    QString modifiedText = text.sliced(0, cursor) + preeditText;

    if (UnicodeFonts::convertLastCharsToFont(modifiedText, numChars, font))
        return modifiedText;

    return {};
}

void FacetUtils::setTextWithoutLinks(const QString& text)
{
    if (text == mTextWithoutLinks)
        return;

    mTextWithoutLinks = text;
    emit textWithoutLinksChanged();
}

void FacetUtils::setEditMention(const QString& mention)
{
    if (mention == mEditMention)
        return;

    mEditMention = mention;
    emit editMentionChanged();
}

void FacetUtils::setEditTag(const QString& tag)
{
    if (tag == mEditTag)
        return;

    mEditTag = tag;
    emit editTagChanged();
}

void FacetUtils::setFirstWebLink(const QString& link)
{
    if (link != mFirstWebLink)
    {
        mFirstWebLink = link;
        emit firstWebLinkChanged();
    }

    if (link.isEmpty())
        setCursorInFirstWebLink(false);
}

void FacetUtils::setFirstWebLink(const ATProto::RichTextMaster::ParsedMatch& linkMatch, int cursor)
{
    setFirstWebLink(linkMatch.mMatch);
    const bool inLink = isCursorInMatch(cursor, linkMatch);
    setCursorInFirstWebLink(inLink);
}

void FacetUtils::setCursorInFirstWebLink(bool inLink)
{
    if (inLink == mCursorInFirstWebLink)
        return;

    mCursorInFirstWebLink = inLink;
    emit cursorInFirstWebLinkChanged();
}

void FacetUtils::setFirstPostLink(const QString& link)
{
    if (link != mFirstPostLink)
    {
        mFirstPostLink = link;
        emit firstPostLinkChanged();
    }

    if (link.isEmpty())
        setCursorInFirstPostLink(false);
}

void FacetUtils::setFirstPostLink(const ATProto::RichTextMaster::ParsedMatch& linkMatch, int cursor)
{
    setFirstPostLink(linkMatch.mMatch);
    const bool inLink = isCursorInMatch(cursor, linkMatch);
    setCursorInFirstPostLink(inLink);
}

void FacetUtils::setCursorInFirstPostLink(bool inLink)
{
    if (inLink == mCursorInFirstPostLink)
        return;

    mCursorInFirstPostLink = inLink;
    emit cursorInFirstPostLinkChanged();
}

void FacetUtils::setFirstFeedLink(const QString& link)
{
    if (link != mFirstFeedLink)
    {
        mFirstFeedLink = link;
        emit firstFeedLinkChanged();
    }

    if (link.isEmpty())
        setCursorInFirstFeedLink(false);
}

void FacetUtils::setFirstFeedLink(const ATProto::RichTextMaster::ParsedMatch& linkMatch, int cursor)
{
    setFirstFeedLink(linkMatch.mMatch);
    const bool inLink = isCursorInMatch(cursor, linkMatch);
    setCursorInFirstFeedLink(inLink);
}

void FacetUtils::setCursorInFirstFeedLink(bool inLink)
{
    if (inLink == mCursorInFirstFeedLink)
        return;

    mCursorInFirstFeedLink = inLink;
    emit cursorInFirstFeedLinkChanged();
}

void FacetUtils::setFirstListLink(const QString& link)
{
    if (link != mFirstListLink)
    {
        mFirstListLink = link;
        emit firstListLinkChanged();
    }

    if (link.isEmpty())
        setCursorInFirstListLink(false);
}

void FacetUtils::setFirstListLink(const ATProto::RichTextMaster::ParsedMatch& linkMatch, int cursor)
{
    setFirstListLink(linkMatch.mMatch);
    const bool inLink = isCursorInMatch(cursor, linkMatch);
    setCursorInFirstListLink(inLink);
}

void FacetUtils::setCursorInFirstListLink(bool inLink)
{
    if (inLink == mCursorInFirstListLink)
        return;

    mCursorInFirstListLink = inLink;
    emit cursorInFirstListLinkChanged();
}

void FacetUtils::setMentions(const QStringList& mentions)
{
    if (mentions == mMentions)
        return;

    mMentions = mentions;
    emit mentionsChanged();
}

void FacetUtils::setWebLinks(const WebLink::List& webLinks)
{
    if (webLinks == mWebLinks)
        return;

    mWebLinks = webLinks;
    emit webLinksChanged();
}

void FacetUtils::setCursorInWebLink(int index)
{
    if (index == mCursorInWebLink)
        return;

    mCursorInWebLink = index;
    emit cursorInWebLinkChanged();
}

void FacetUtils::setEmbeddedLinks(const WebLink::List& embeddedLinks)
{
    if (embeddedLinks == mEmbeddedLinks)
        return;

    mEmbeddedLinks = embeddedLinks;
    signalEmbeddedLinksUpdated();
}

void FacetUtils::setCursorInEmbeddedLink(int index)
{
    if (index == mCursorInEmbeddedLink)
        return;

    mCursorInEmbeddedLink = index;
    emit cursorInEmbeddedLinkChanged();
}

}
