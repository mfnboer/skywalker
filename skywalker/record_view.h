// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "generator_view.h"
#include "image_view.h"
#include "labeler.h"
#include "language_utils.h"
#include "list_view.h"
#include "normalized_word_index.h"
#include "profile.h"
#include "starter_pack.h"
#include <atproto/lib/lexicon/app_bsky_embed.h>
#include <QtQmlIntegration>

namespace Skywalker
{

class RecordView : public NormalizedWordIndex
{
    Q_GADGET
    Q_PROPERTY(QString postUri READ getUri FINAL)
    Q_PROPERTY(QString postPlainText READ getText FINAL)
    Q_PROPERTY(QString postTextFormatted READ getFormattedText FINAL)
    Q_PROPERTY(BasicProfile author READ getAuthor FINAL)
    Q_PROPERTY(QDateTime postDateTime READ getIndexedAt FINAL)
    Q_PROPERTY(bool postIsReply READ isReply FINAL)
    Q_PROPERTY(QEnums::TripleBool postIsThread READ isThread FINAL)
    Q_PROPERTY(bool postIsThreadReply READ isThreadReply FINAL)
    Q_PROPERTY(BasicProfile replyToAuthor READ getReplyToAuthor FINAL)
    Q_PROPERTY(bool hasUnknownEmbed READ hasUnknownEmbed FINAL)
    Q_PROPERTY(QString unknownEmbedType READ getUnknownEmbedType FINAL)
    Q_PROPERTY(QList<ImageView> images READ getImages FINAL)
    Q_PROPERTY(LanguageList languages READ getLanguages FINAL)
    Q_PROPERTY(ContentLabelList contentLabels READ getContentLabels FINAL)
    Q_PROPERTY(QEnums::ContentVisibility contentVisibility READ getContentVisibility FINAL)
    Q_PROPERTY(QString contentWarning READ getContentWarning FINAL)
    Q_PROPERTY(QEnums::MutedPostReason mutedReason READ getMutedReason FINAL)
    Q_PROPERTY(QVariant video READ getVideo FINAL)
    Q_PROPERTY(QVariant external READ getExternal FINAL)
    Q_PROPERTY(bool notFound READ getNotFound FINAL)
    Q_PROPERTY(bool blocked READ getBlocked FINAL)
    Q_PROPERTY(bool detached READ getDetached FINAL)
    Q_PROPERTY(QString detachedByDid READ getDetachedByDid FINAL)
    Q_PROPERTY(QString detachedPostUri READ getDetachedPostUri FINAL)
    Q_PROPERTY(bool notSupported READ getNotSupported FINAL)
    Q_PROPERTY(QString unsupportedType READ getUnsupportedType FINAL)
    Q_PROPERTY(bool available READ getAvailable FINAL)
    Q_PROPERTY(bool feedAvailable READ getFeedAvailable FINAL)
    Q_PROPERTY(GeneratorView feed READ getFeed FINAL)
    Q_PROPERTY(bool listAvailable READ getListAvailable FINAL)
    Q_PROPERTY(ListView list READ getList FINAL)
    Q_PROPERTY(bool labelerAvailable READ getLabelerAvailable FINAL)
    Q_PROPERTY(LabelerView labeler READ getLabeler FINAL)
    Q_PROPERTY(bool starterPackAvailable READ getStarterPackAvailable FINAL)
    Q_PROPERTY(StarterPackViewBasic starterPack READ getStarterPack FINAL)
    QML_VALUE_TYPE(recordview)

public:
    using Ptr = std::unique_ptr<RecordView>;
    using SharedPtr = std::shared_ptr<RecordView>;

    static SharedPtr makeDetachedRecord(const QString postUri);

    RecordView() = default;
    explicit RecordView(const ATProto::AppBskyEmbed::RecordView& view);

    Q_INVOKABLE bool isNull() const { return !mValid; }
    QString getUri() const;
    QString getCid() const;
    QString getText() const override;
    QString getFormattedText() const;
    BasicProfile getAuthor() const;
    QString getAuthorDid() const override { return getAuthor().getDid(); }
    QDateTime getIndexedAt() const;
    bool hasUnknownEmbed() const;
    QString getUnknownEmbedType() const;
    QList<ImageView> getImages() const override;
    QVariant getVideo() const;
    VideoView::Ptr getVideoView() const override;

    QVariant getExternal() const;
    ExternalView::Ptr getExternalView() const override;
    const ContentLabelList& getContentLabels() const;
    const std::vector<ATProto::ComATProtoLabel::Label::SharedPtr>& getLabels() const;
    const ContentLabelList& getLabelsIncludingAuthorLabels() const;
    bool isReply() const;
    QString getReplyToAuthorDid() const;
    BasicProfile getReplyToAuthor() const;
    QString getReplyRootAuthorDid() const;
    const LanguageList& getLanguages() const;
    std::vector<QString> getHashtags() const override;
    std::vector<QString> getWebLinks() const override;
    QEnums::ContentVisibility getContentVisibility() const { return mPrivate->mContentVisibility; }
    const QString& getContentWarning() const { return mPrivate->mContentWarning; }
    QEnums::MutedPostReason getMutedReason() const { return mPrivate->mMutedReason; }
    QEnums::TripleBool isThread() const;
    bool isThreadReply() const;

    bool getNotFound() const { return mPrivate->mNotFound; }
    bool getBlocked() const { return mPrivate->mBlocked; }
    bool getDetached() const { return mPrivate->mDetached; }
    const QString& getDetachedByDid() const { return mPrivate->mDetachedByDid; }
    const QString& getDetachedPostUri() const { return mPrivate->mDetachedPostUri; }
    bool getNotSupported() const { return mPrivate->mNotSupported; }
    const QString& getUnsupportedType() const { return mPrivate->mUnsupportedType; }
    bool getAvailable() const { return mPrivate->mRecord != nullptr; }
    bool getFeedAvailable()  const { return mPrivate->mFeed != nullptr; }
    GeneratorView getFeed() const;
    bool getListAvailable() const { return mPrivate->mList != nullptr; }
    ListView getList() const;
    bool getLabelerAvailable() const { return mPrivate->mLabeler != nullptr; }
    LabelerView getLabeler() const;
    bool getStarterPackAvailable() const { return mPrivate->mStarterPack != nullptr; }
    StarterPackViewBasic getStarterPack() const;

    void setContentVisibility(QEnums::ContentVisibility visibility) { mPrivate->mContentVisibility = visibility; }
    void setContentWarning(const QString& warning) { mPrivate->mContentWarning = warning; }
    void setMutedReason(const QEnums::MutedPostReason mutedReason) { mPrivate->mMutedReason = mutedReason; }
    void setMutedReason(const IMatchWords& mutedWords);

private:
    ATProto::AppBskyEmbed::EmbedView::SharedPtr getEmbedView(ATProto::AppBskyEmbed::EmbedViewType embedViewType) const;

    bool mValid = false;

    struct PrivateData
    {
        ATProto::AppBskyEmbed::RecordViewRecord::SharedPtr mRecord;
        ATProto::AppBskyFeed::GeneratorView::SharedPtr mFeed;
        ATProto::AppBskyGraph::ListView::SharedPtr mList;
        ATProto::AppBskyLabeler::LabelerView::SharedPtr mLabeler;
        ATProto::AppBskyGraph::StarterPackViewBasic::SharedPtr mStarterPack;
        bool mNotFound = false;
        bool mBlocked = false;
        bool mDetached = false;
        QString mDetachedByDid;
        QString mDetachedPostUri;
        bool mNotSupported = false;
        QString mUnsupportedType;
        QEnums::ContentVisibility mContentVisibility = QEnums::CONTENT_VISIBILITY_HIDE_POST;
        QString mContentWarning = "NOT INITIALIZED";
        QEnums::MutedPostReason mMutedReason = QEnums::MUTED_POST_NONE;
        LanguageList mLanguages;
        std::optional<ContentLabelList> mContentLabels;
        std::optional<ContentLabelList> mLabelsIncludingAuthorLabels;
    };
    std::shared_ptr<PrivateData> mPrivate = std::make_shared<PrivateData>();
};

}

Q_DECLARE_METATYPE(::Skywalker::RecordView)
