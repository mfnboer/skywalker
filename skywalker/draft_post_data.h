// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "generator_view.h"
#include "image_view.h"
#include "list_view.h"
#include "profile.h"
#include "tenor_gif.h"
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class DraftPostData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged FINAL)
    Q_PROPERTY(QList<ImageView> images READ images WRITE setImages NOTIFY imagesChanged FINAL)
    Q_PROPERTY(QString replyToUri READ replyToUri WRITE setReplyToUri NOTIFY replyToUriChanged FINAL)
    Q_PROPERTY(QString replyToCid READ replyToCid WRITE setReplyToCid NOTIFY replyToCidChanged FINAL)
    Q_PROPERTY(QString replyRootUri READ replyRootUri WRITE setReplyRootUri NOTIFY replyRootUriChanged FINAL)
    Q_PROPERTY(QString replyRootCid READ replyRootCid WRITE setReplyRootCid NOTIFY replyRootCidChanged FINAL)
    Q_PROPERTY(BasicProfile replyToAuthor READ replyToAuthor WRITE setReplyToAuthor NOTIFY replyToAuthorChanged FINAL)
    Q_PROPERTY(QString replyToText READ replyToText WRITE setReplyToText NOTIFY replyToTextChanged FINAL)
    Q_PROPERTY(QDateTime replyToDateTime READ replyToDateTime WRITE setReplyToDateTime NOTIFY replyToDateTimeChanged FINAL)
    Q_PROPERTY(bool openAsQuotePost READ openAsQuotePost WRITE setOpenAsQuotePost NOTIFY openAsQuotePostChanged FINAL)
    Q_PROPERTY(QString quoteUri READ quoteUri WRITE setQuoteUri NOTIFY quoteUriChanged FINAL)
    Q_PROPERTY(QString quoteCid READ quoteCid WRITE setQuoteCid NOTIFY quoteCidChanged FINAL)
    Q_PROPERTY(BasicProfile quoteAuthor READ quoteAuthor WRITE setQuoteAuthor NOTIFY quoteAuthorChanged FINAL)
    Q_PROPERTY(QString quoteText READ quoteText WRITE setQuoteText NOTIFY quoteTextChanged FINAL)
    Q_PROPERTY(QDateTime quoteDateTime READ quoteDateTime WRITE setQuoteDateTime NOTIFY quoteDateTimeChanged FINAL)
    Q_PROPERTY(GeneratorView quoteFeed READ quoteFeed WRITE setQuoteFeed NOTIFY quoteFeedChanged FINAL)
    Q_PROPERTY(ListView quoteList READ quoteList WRITE setQuoteList NOTIFY quoteListChanged FINAL)
    Q_PROPERTY(TenorGif gif READ gif WRITE setGif NOTIFY gifChanged FINAL)
    Q_PROPERTY(QStringList labels READ labels WRITE setLabels NOTIFY labelsChanged FINAL)
    Q_PROPERTY(bool restrictReplies READ restrictReplies WRITE setRestrictReplies NOTIFY restrictRepliesChanged FINAL)
    Q_PROPERTY(bool allowMention READ allowMention WRITE setAllowMention NOTIFY allowMentionChanged FINAL)
    Q_PROPERTY(bool allowFollowing READ allowFollowing WRITE setAllowFollowing NOTIFY allowFollowingChanged FINAL)
    Q_PROPERTY(QStringList allowLists READ allowLists WRITE setAllowLists NOTIFY allowListsChanged FINAL)
    Q_PROPERTY(QString draftPostFileName READ draftPostFileName WRITE setDraftPostFileName NOTIFY draftPostFileNameChanged FINAL)
    QML_ELEMENT

public:
    explicit DraftPostData(QObject* parent = nullptr) : QObject(parent) {}

    QString text() const;
    void setText(const QString &newText);
    QList<ImageView> images() const;
    void setImages(const QList<ImageView> &newImages);
    QString replyToUri() const;
    void setReplyToUri(const QString &newReplyToUri);
    QString replyToCid() const;
    void setReplyToCid(const QString &newReplyToCid);
    QString replyRootUri() const;
    void setReplyRootUri(const QString &newReplyRootUri);
    QString replyRootCid() const;
    void setReplyRootCid(const QString &newReplyRootCid);
    BasicProfile replyToAuthor() const;
    void setReplyToAuthor(const BasicProfile &newReplyToAuthor);
    QString replyToText() const;
    void setReplyToText(const QString &newReplyToText);
    QDateTime replyToDateTime() const;
    void setReplyToDateTime(const QDateTime &newReplyToDateTime);
    bool openAsQuotePost() const;
    void setOpenAsQuotePost(bool newOpenAsQuotePost);
    QString quoteUri() const;
    void setQuoteUri(const QString &newQuoteUri);
    QString quoteCid() const;
    void setQuoteCid(const QString &newQuoteCid);
    BasicProfile quoteAuthor() const;
    void setQuoteAuthor(const BasicProfile &newQuoteAuthor);
    QString quoteText() const;
    void setQuoteText(const QString &newQuoteText);
    QDateTime quoteDateTime() const;
    void setQuoteDateTime(const QDateTime &newQuoteDateTime);
    GeneratorView quoteFeed() const;
    void setQuoteFeed(const GeneratorView &newQuoteFeed);
    ListView quoteList() const;
    void setQuoteList(const ListView& newQuoteList);
    const TenorGif& gif() const;
    void setGif(const TenorGif &newGif);
    QStringList labels() const;
    void setLabels(const QStringList &newLabels);
    bool restrictReplies() const;
    void setRestrictReplies(bool newRestrictReplies);
    bool allowMention() const;
    void setAllowMention(bool newAllowMention);
    bool allowFollowing() const;
    void setAllowFollowing(bool newAllowFollowing);
    QStringList allowLists() const;
    void setAllowLists(const QStringList &newAllowLists);
    QString draftPostFileName() const;
    void setDraftPostFileName(const QString &newDraftPostFileName);

signals:
    void textChanged();
    void imagesChanged();
    void replyToUriChanged();
    void replyToCidChanged();
    void replyRootUriChanged();
    void replyRootCidChanged();
    void replyToAuthorChanged();
    void replyToTextChanged();
    void replyToDateTimeChanged();
    void quoteUriChanged();
    void quoteCidChanged();
    void quoteAuthorChanged();
    void quoteTextChanged();
    void quoteDateTimeChanged();
    void quoteFeedChanged();
    void quoteListChanged();
    void gifChanged();
    void labelsChanged();
    void restrictRepliesChanged();
    void allowMentionChanged();
    void allowFollowingChanged();
    void allowListsChanged();
    void openAsQuotePostChanged();
    void draftPostFileNameChanged();

private:
    QString mText;
    QList<ImageView> mImages;
    QString mReplyToUri;
    QString mReplyToCid;
    QString mReplyRootUri;
    QString mReplyRootCid;
    BasicProfile mReplyToAuthor;
    QString mReplyToText;
    QDateTime mReplyToDateTime;
    bool mOpenAsQuotePost = false;
    QString mQuoteUri;
    QString mQuoteCid;
    BasicProfile mQuoteAuthor;
    QString mQuoteText;
    QDateTime mQuoteDateTime;
    GeneratorView mQuoteFeed;
    ListView mQuoteList;
    TenorGif mGif;
    QStringList mLabels;
    bool mRestrictReplies;
    bool mAllowMention;
    bool mAllowFollowing;
    QStringList mAllowLists;
    QString mDraftPostFileName;
};

inline QString DraftPostData::text() const
{
    return mText;
}

inline void DraftPostData::setText(const QString &newText)
{
    if (mText == newText)
        return;
    mText = newText;
    emit textChanged();
}

inline QList<ImageView> DraftPostData::images() const
{
    return mImages;
}

inline void DraftPostData::setImages(const QList<ImageView> &newImages)
{
    mImages = newImages;
    emit imagesChanged();
}

inline QString DraftPostData::replyToUri() const
{
    return mReplyToUri;
}

inline void DraftPostData::setReplyToUri(const QString &newReplyToUri)
{
    if (mReplyToUri == newReplyToUri)
        return;
    mReplyToUri = newReplyToUri;
    emit replyToUriChanged();
}

inline QString DraftPostData::replyToCid() const
{
    return mReplyToCid;
}

inline void DraftPostData::setReplyToCid(const QString &newReplyToCid)
{
    if (mReplyToCid == newReplyToCid)
        return;
    mReplyToCid = newReplyToCid;
    emit replyToCidChanged();
}

inline QString DraftPostData::replyRootUri() const
{
    return mReplyRootUri;
}

inline void DraftPostData::setReplyRootUri(const QString &newReplyRootUri)
{
    if (mReplyRootUri == newReplyRootUri)
        return;
    mReplyRootUri = newReplyRootUri;
    emit replyRootUriChanged();
}

inline QString DraftPostData::replyRootCid() const
{
    return mReplyRootCid;
}

inline void DraftPostData::setReplyRootCid(const QString &newReplyRootCid)
{
    if (mReplyRootCid == newReplyRootCid)
        return;
    mReplyRootCid = newReplyRootCid;
    emit replyRootCidChanged();
}

inline BasicProfile DraftPostData::replyToAuthor() const
{
    return mReplyToAuthor;
}

inline void DraftPostData::setReplyToAuthor(const BasicProfile &newReplyToAuthor)
{
    if (mReplyToAuthor.getDid() == newReplyToAuthor.getDid())
        return;
    mReplyToAuthor = newReplyToAuthor;
    emit replyToAuthorChanged();
}

inline QString DraftPostData::replyToText() const
{
    return mReplyToText;
}

inline void DraftPostData::setReplyToText(const QString &newReplyToText)
{
    if (mReplyToText == newReplyToText)
        return;
    mReplyToText = newReplyToText;
    emit replyToTextChanged();
}

inline QDateTime DraftPostData::replyToDateTime() const
{
    return mReplyToDateTime;
}

inline void DraftPostData::setReplyToDateTime(const QDateTime &newReplyToDateTime)
{
    if (mReplyToDateTime == newReplyToDateTime)
        return;
    mReplyToDateTime = newReplyToDateTime;
    emit replyToDateTimeChanged();
}

inline QString DraftPostData::quoteUri() const
{
    return mQuoteUri;
}

inline void DraftPostData::setQuoteUri(const QString &newQuoteUri)
{
    if (mQuoteUri == newQuoteUri)
        return;
    mQuoteUri = newQuoteUri;
    emit quoteUriChanged();
}

inline QString DraftPostData::quoteCid() const
{
    return mQuoteCid;
}

inline void DraftPostData::setQuoteCid(const QString &newQuoteCid)
{
    if (mQuoteCid == newQuoteCid)
        return;
    mQuoteCid = newQuoteCid;
    emit quoteCidChanged();
}

inline BasicProfile DraftPostData::quoteAuthor() const
{
    return mQuoteAuthor;
}

inline void DraftPostData::setQuoteAuthor(const BasicProfile &newQuoteAuthor)
{
    if (mQuoteAuthor.getDid() == newQuoteAuthor.getDid())
        return;
    mQuoteAuthor = newQuoteAuthor;
    emit quoteAuthorChanged();
}

inline QString DraftPostData::quoteText() const
{
    return mQuoteText;
}

inline void DraftPostData::setQuoteText(const QString &newQuoteText)
{
    if (mQuoteText == newQuoteText)
        return;
    mQuoteText = newQuoteText;
    emit quoteTextChanged();
}

inline QDateTime DraftPostData::quoteDateTime() const
{
    return mQuoteDateTime;
}

inline void DraftPostData::setQuoteDateTime(const QDateTime &newQuoteDateTime)
{
    if (mQuoteDateTime == newQuoteDateTime)
        return;
    mQuoteDateTime = newQuoteDateTime;
    emit quoteDateTimeChanged();
}

inline GeneratorView DraftPostData::quoteFeed() const
{
    return mQuoteFeed;
}

inline void DraftPostData::setQuoteFeed(const GeneratorView &newQuoteFeed)
{
    if (mQuoteFeed.getUri() == newQuoteFeed.getUri())
        return;
    mQuoteFeed = newQuoteFeed;
    emit quoteFeedChanged();
}

inline ListView DraftPostData::quoteList() const
{
    return mQuoteList;
}

inline void DraftPostData::setQuoteList(const ListView& newQuoteList)
{
    if (mQuoteList.getUri() == newQuoteList.getUri())
        return;
    mQuoteList = newQuoteList;
    emit quoteListChanged();
}

inline const TenorGif& DraftPostData::gif() const
{
    return mGif;
}

inline void DraftPostData::setGif(const TenorGif &newGif)
{
    mGif = newGif;
    emit gifChanged();
}

inline QStringList DraftPostData::labels() const
{
    return mLabels;
}

inline void DraftPostData::setLabels(const QStringList &newLabels)
{
    if (mLabels == newLabels)
        return;
    mLabels = newLabels;
    emit labelsChanged();
}

inline bool DraftPostData::restrictReplies() const
{
    return mRestrictReplies;
}

inline void DraftPostData::setRestrictReplies(bool newRestrictReplies)
{
    if (mRestrictReplies == newRestrictReplies)
        return;
    mRestrictReplies = newRestrictReplies;
    emit restrictRepliesChanged();
}

inline bool DraftPostData::allowMention() const
{
    return mAllowMention;
}

inline void DraftPostData::setAllowMention(bool newAllowMention)
{
    if (mAllowMention == newAllowMention)
        return;
    mAllowMention = newAllowMention;
    emit allowMentionChanged();
}

inline bool DraftPostData::allowFollowing() const
{
    return mAllowFollowing;
}

inline void DraftPostData::setAllowFollowing(bool newAllowFollowing)
{
    if (mAllowFollowing == newAllowFollowing)
        return;
    mAllowFollowing = newAllowFollowing;
    emit allowFollowingChanged();
}

inline QStringList DraftPostData::allowLists() const
{
    return mAllowLists;
}

inline void DraftPostData::setAllowLists(const QStringList &newAllowLists)
{
    if (mAllowLists == newAllowLists)
        return;
    mAllowLists = newAllowLists;
    emit allowListsChanged();
}

inline QString DraftPostData::draftPostFileName() const
{
    return mDraftPostFileName;
}

inline void DraftPostData::setDraftPostFileName(const QString &newDraftPostFileName)
{
    if (mDraftPostFileName == newDraftPostFileName)
        return;
    mDraftPostFileName = newDraftPostFileName;
    emit draftPostFileNameChanged();
}

inline bool DraftPostData::openAsQuotePost() const
{
    return mOpenAsQuotePost;
}

inline void DraftPostData::setOpenAsQuotePost(bool newOpenAsQuotePost)
{
    if (mOpenAsQuotePost == newOpenAsQuotePost)
        return;
    mOpenAsQuotePost = newOpenAsQuotePost;
    emit openAsQuotePostChanged();
}

}
