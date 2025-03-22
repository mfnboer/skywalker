// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "facet_highlighter.h"
#include "text_differ.h"
#include "presence.h"
#include "enums.h"
#include "wrapped_skywalker.h"
#include "web_link.h"

namespace Skywalker {

class FacetUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    Q_PROPERTY(QString textWithoutLinks READ getTextWithoutLinks WRITE setTextWithoutLinks NOTIFY textWithoutLinksChanged FINAL)
    Q_PROPERTY(QString editMention READ getEditMention WRITE setEditMention NOTIFY editMentionChanged FINAL)
    Q_PROPERTY(QString editTag READ getEditTag WRITE setEditTag NOTIFY editTagChanged FINAL)
    Q_PROPERTY(QString firstWebLink READ getFirstWebLink WRITE setFirstWebLink NOTIFY firstWebLinkChanged FINAL)
    Q_PROPERTY(bool cursorInFirstWebLink READ isCursorInFirstWebLink WRITE setCursorInFirstWebLink NOTIFY cursorInFirstWebLinkChanged FINAL)
    Q_PROPERTY(QString firstPostLink READ getFirstPostLink WRITE setFirstPostLink NOTIFY firstPostLinkChanged FINAL)
    Q_PROPERTY(bool cursorInFirstPostLink READ isCursorInFirstPostLink WRITE setCursorInFirstPostLink NOTIFY cursorInFirstPostLinkChanged FINAL)
    Q_PROPERTY(QString firstFeedLink READ getFirstFeedLink WRITE setFirstFeedLink NOTIFY firstFeedLinkChanged FINAL)
    Q_PROPERTY(bool cursorInFirstFeedLink READ isCursorInFirstFeedLink WRITE setCursorInFirstFeedLink NOTIFY cursorInFirstFeedLinkChanged FINAL)
    Q_PROPERTY(QString firstListLink READ getFirstListLink WRITE setFirstListLink NOTIFY firstListLinkChanged FINAL)
    Q_PROPERTY(bool cursorInFirstListLink READ isCursorInFirstListLink WRITE setCursorInFirstListLink NOTIFY cursorInFirstListLinkChanged FINAL)
    Q_PROPERTY(WebLink::List webLinks READ getWebLinks WRITE setWebLinks NOTIFY webLinksChanged FINAL)
    Q_PROPERTY(int cursorInWebLink READ getCursorInWebLink WRITE setCursorInWebLink NOTIFY cursorInWebLinkChanged FINAL)
    Q_PROPERTY(WebLink::List embeddedLinks READ getEmbeddedLinks WRITE setEmbeddedLinks NOTIFY embeddedLinksChanged FINAL)
    Q_PROPERTY(int cursorInEmbeddedLink READ getCursorInEmbeddedLink WRITE setCursorInEmbeddedLink NOTIFY cursorInEmbeddedLinkChanged FINAL)
    QML_ELEMENT

public:
    explicit FacetUtils(QObject* parent = nullptr);

    Q_INVOKABLE void setHighlightDocument(QQuickTextDocument* doc, const QString& highlightColor,
                                          const QString& errorColor,
                                          int maxLength = -1, const QString& lengthExceededColor = {});
    Q_INVOKABLE void setHighLightMaxLength(int maxLength);
    Q_INVOKABLE void extractMentionsAndLinks(const QString& text,const QString& preeditText, int cursor);
    Q_INVOKABLE bool checkMisleadingEmbeddedLinks() const;
    Q_INVOKABLE WebLink makeWebLink(const QString& name, const QString& link, int startIndex, int endIndex) const;
    Q_INVOKABLE void addEmbeddedLink(const WebLink& link);
    Q_INVOKABLE void updatedEmbeddedLink(int linkIndex, const WebLink& link);
    Q_INVOKABLE void removeEmbeddedLink(int linkIndex);
    // Make updates due to cursor moving to a new position
    Q_INVOKABLE void updateCursor(int cursor);
    // Make updates due to changed text
    Q_INVOKABLE void updateText(const QString& prevText, const QString& text);
    // Returns a new text up to cursor with the last type char transformed into font.
    // Returns null string if last typed char was not a transformable char.
    Q_INVOKABLE QString applyFontToLastTypedChars(const QString& text,const QString& preeditText,
                                                  int cursor, int numChars, QEnums::FontType font);
    Q_INVOKABLE int getEditMentionIndex() const { return mEditMentionIndex; }
    Q_INVOKABLE int getEditTagIndex() const { return mEditTagIndex; }
    Q_INVOKABLE int getLinkShorteningReduction() const { return mLinkShorteningReduction; }

    const QString& getTextWithoutLinks() const { return mTextWithoutLinks; }
    void setTextWithoutLinks(const QString& text);
    const QString& getEditMention() const { return mEditMention; }
    void setEditMention(const QString& mention);
    const QString& getEditTag() const { return mEditTag; }
    void setEditTag(const QString& tag);
    const QString& getFirstWebLink() const { return mFirstWebLink; }
    void setFirstWebLink(const QString& link);
    void setFirstWebLink(const ATProto::RichTextMaster::ParsedMatch& linkMatch, int cursor);
    bool isCursorInFirstWebLink() const { return mCursorInFirstWebLink; }
    void setCursorInFirstWebLink(bool inLink);
    const QString& getFirstPostLink() const { return mFirstPostLink; }
    void setFirstPostLink(const QString& link);
    void setFirstPostLink(const ATProto::RichTextMaster::ParsedMatch& linkMatch, int cursor);
    bool isCursorInFirstPostLink() const { return mCursorInFirstPostLink; }
    void setCursorInFirstPostLink(bool inLink);
    const QString& getFirstFeedLink() const { return mFirstFeedLink; }
    void setFirstFeedLink(const QString& link);
    void setFirstFeedLink(const ATProto::RichTextMaster::ParsedMatch& linkMatch, int cursor);
    bool isCursorInFirstFeedLink() const { return mCursorInFirstFeedLink; }
    void setCursorInFirstFeedLink(bool inLink);
    const QString& getFirstListLink() const { return mFirstListLink; }
    void setFirstListLink(const QString& link);
    void setFirstListLink(const ATProto::RichTextMaster::ParsedMatch& linkMatch, int cursor);
    bool isCursorInFirstListLink() const { return mCursorInFirstListLink; }
    void setCursorInFirstListLink(bool inLink);
    const WebLink::List& getWebLinks() const { return mWebLinks; }
    void setWebLinks(const WebLink::List& webLinks);
    int getCursorInWebLink() const { return mCursorInWebLink; }
    void setCursorInWebLink(int index);
    const WebLink::List& getEmbeddedLinks() const { return mEmbeddedLinks; }
    void setEmbeddedLinks(const WebLink::List& embeddedLinks);
    int getCursorInEmbeddedLink() const { return mCursorInEmbeddedLink; }
    void setCursorInEmbeddedLink(int index);

signals:
    void textWithoutLinksChanged();
    void editMentionChanged();
    void editTagChanged();
    void firstWebLinkChanged();
    void cursorInFirstWebLinkChanged();
    void firstPostLinkChanged();
    void cursorInFirstPostLinkChanged();
    void firstFeedLinkChanged();
    void cursorInFirstFeedLinkChanged();
    void firstListLinkChanged();
    void cursorInFirstListLinkChanged();
    void webLinksChanged();
    void cursorInWebLinkChanged();
    void embeddedLinksChanged();
    void cursorInEmbeddedLinkChanged();

private:
    void updateEmbeddedLinksInsertedText(const TextDiffer::Result& diff, const QString& text);
    void updateEmbeddedLinksDeletedText(const TextDiffer::Result& diff);
    void updateEmbeddedLinksUpdatedText(const TextDiffer::Result& diff, const QString& text);
    bool facetOverlapsWithEmbeddedLink(const ATProto::RichTextMaster::ParsedMatch& facet) const;

    QString mEditMention; // Mention currently being edited (without @-symbol)
    int mEditMentionIndex = 0;
    QString mEditTag; // Tag currently being edited (without #-symbol)
    int mEditTagIndex = 0;
    QString mFirstPostLink; // HTTPS link to a post
    bool mCursorInFirstPostLink = false;
    QString mFirstFeedLink; // HTTPS link to feed generator
    bool mCursorInFirstFeedLink = false;
    QString mFirstListLink; // HTTPS link to list ivew
    bool mCursorInFirstListLink = false;
    QString mFirstWebLink;
    bool mCursorInFirstWebLink = false;
    int mLinkShorteningReduction = 0;
    QString mTextWithoutLinks;
    WebLink::List mWebLinks;
    int mCursorInWebLink = -1;
    WebLink::List mEmbeddedLinks;
    int mCursorInEmbeddedLink = -1;

    FacetHighlighter mFacetHighlighter;
};

}
