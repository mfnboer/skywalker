// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "generator_view.h"
#include "image_view.h"
#include "labeler.h"
#include "language_utils.h"
#include "list_view.h"
#include "profile.h"
#include "record_word_index.h"
#include "starter_pack.h"
#include "text_meta_info.h"
#include <atproto/lib/lexicon/app_bsky_embed.h>
#include <QtQmlIntegration>

namespace Skywalker {

class RecordView
{
    Q_GADGET
    Q_PROPERTY(QString postUri READ getUri CONSTANT FINAL)
    Q_PROPERTY(QString postPlainText READ getText CONSTANT FINAL)
    Q_PROPERTY(QString postTextFormatted READ getFormattedText CONSTANT FINAL)
    Q_PROPERTY(TextMetaInfo postTextMetaInfo READ getTextMetaInfo CONSTANT FINAL)
    Q_PROPERTY(BasicProfile author READ getAuthor CONSTANT FINAL)
    Q_PROPERTY(QDateTime postDateTime READ getIndexedAt CONSTANT FINAL)
    Q_PROPERTY(bool postIsReply READ isReply CONSTANT FINAL)
    Q_PROPERTY(QEnums::TripleBool postIsThread READ isThread CONSTANT FINAL)
    Q_PROPERTY(bool postIsThreadReply READ isThreadReply CONSTANT FINAL)
    Q_PROPERTY(BasicProfile replyToAuthor READ getReplyToAuthor CONSTANT FINAL)
    Q_PROPERTY(bool hasUnknownEmbed READ hasUnknownEmbed CONSTANT FINAL)
    Q_PROPERTY(QString unknownEmbedType READ getUnknownEmbedType CONSTANT FINAL)
    Q_PROPERTY(QList<ImageView> images READ getImages CONSTANT FINAL)
    Q_PROPERTY(LanguageList languages READ getLanguages CONSTANT FINAL)
    Q_PROPERTY(ContentLabelList contentLabels READ getContentLabels CONSTANT FINAL)
    Q_PROPERTY(QEnums::ContentVisibility contentVisibility READ getContentVisibility CONSTANT FINAL)
    Q_PROPERTY(QString contentWarning READ getContentWarning CONSTANT FINAL)
    Q_PROPERTY(BasicProfile contentLabeler READ getContentLabeler CONSTANT FINAL)
    Q_PROPERTY(QEnums::MutedPostReason mutedReason READ getMutedReason CONSTANT FINAL)
    Q_PROPERTY(QVariant video READ getVideo CONSTANT FINAL)
    Q_PROPERTY(QVariant external READ getExternal CONSTANT FINAL)
    Q_PROPERTY(bool notFound READ getNotFound CONSTANT FINAL)
    Q_PROPERTY(bool blocked READ getBlocked CONSTANT FINAL)
    Q_PROPERTY(BlockedAuthor blockedAuthor READ getBlockedAuthor CONSTANT FINAL)
    Q_PROPERTY(bool detached READ getDetached CONSTANT FINAL)
    Q_PROPERTY(QString detachedByDid READ getDetachedByDid CONSTANT FINAL)
    Q_PROPERTY(QString detachedPostUri READ getDetachedPostUri CONSTANT FINAL)
    Q_PROPERTY(bool notSupported READ getNotSupported CONSTANT FINAL)
    Q_PROPERTY(QString unsupportedType READ getUnsupportedType CONSTANT FINAL)
    Q_PROPERTY(bool available READ getAvailable CONSTANT FINAL)
    Q_PROPERTY(bool feedAvailable READ getFeedAvailable CONSTANT FINAL)
    Q_PROPERTY(GeneratorView feed READ getFeed CONSTANT FINAL)
    Q_PROPERTY(bool listAvailable READ getListAvailable CONSTANT FINAL)
    Q_PROPERTY(ListView list READ getList CONSTANT FINAL)
    Q_PROPERTY(bool labelerAvailable READ getLabelerAvailable CONSTANT FINAL)
    Q_PROPERTY(LabelerView labeler READ getLabeler CONSTANT FINAL)
    Q_PROPERTY(bool starterPackAvailable READ getStarterPackAvailable CONSTANT FINAL)
    Q_PROPERTY(StarterPackViewBasic starterPack READ getStarterPack CONSTANT FINAL)
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
    QString getText() const;
    QString getFormattedText() const;
    TextMetaInfo getTextMetaInfo() const;
    BasicProfile getAuthor() const;
    QString getAuthorDid() const { return getAuthor().getDid(); }
    QDateTime getIndexedAt() const;
    bool hasUnknownEmbed() const;
    QString getUnknownEmbedType() const;
    QList<ImageView> getImages() const;
    QVariant getVideo() const;
    VideoView::Ptr getVideoView() const;

    QVariant getExternal() const;
    ExternalView::Ptr getExternalView() const;
    const ContentLabelList& getContentLabels() const;
    const std::vector<ATProto::ComATProtoLabel::Label::SharedPtr>& getLabels() const;
    const ContentLabelList& getLabelsIncludingAuthorLabels() const;
    bool isReply() const;
    QString getReplyToAuthorDid() const;
    BasicProfile getReplyToAuthor() const;
    QString getReplyRootAuthorDid() const;
    const LanguageList& getLanguages() const;
    QEnums::ContentVisibility getContentVisibility() const { return mPrivate->mContentVisibility; }
    const QString& getContentWarning() const { return mPrivate->mContentWarning; }
    const BasicProfile& getContentLabeler() const { return mPrivate->mContentLabeler; }
    QEnums::MutedPostReason getMutedReason() const { return mPrivate->mMutedReason; }
    QEnums::TripleBool isThread() const;
    bool isThreadReply() const;

    bool getNotFound() const { return mPrivate->mNotFound; }
    const QString& getNotFoundUri() const { return mPrivate->mNotFoundUri; }
    bool getBlocked() const { return mPrivate->mBlocked; }
    const BlockedAuthor& getBlockedAuthor() const { return mPrivate->mBlockedAuthor; }
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

    const RecordWordIndex* getRecordWordIndex() const { return mPrivate->mRecordWordIndex.get(); }

    void setContentVisibility(QEnums::ContentVisibility visibility) { mPrivate->mContentVisibility = visibility; }
    void setContentWarning(const QString& warning) { mPrivate->mContentWarning = warning; }
    void setContentLabeler(const BasicProfile& labeler) { mPrivate->mContentLabeler = labeler; }
    void setMutedReason(const QEnums::MutedPostReason mutedReason) { mPrivate->mMutedReason = mutedReason; }
    void setMutedReason(const IMatchWords& mutedWords);

    void setFormattedText(const QString& text) { mPrivate->mFormattedText = text; }

    void setImages(const QList<ImageView>& images);
    void setVideo(const VideoView& video);
    void setExternal(const ExternalView& external);

private:
    ATProto::AppBskyEmbed::EmbedView::SharedPtr getEmbedView(ATProto::AppBskyEmbed::EmbedViewType embedViewType) const;
    bool hasFacets() const;

    bool mValid = false;

    // Optimal field order as suggested by clang-analyzer
    struct PrivateData
    {
        RecordWordIndex::Ptr mRecordWordIndex;
        ATProto::AppBskyEmbed::RecordViewRecord::SharedPtr mRecord;
        ATProto::AppBskyFeed::GeneratorView::SharedPtr mFeed;
        ATProto::AppBskyGraph::ListView::SharedPtr mList;
        ATProto::AppBskyLabeler::LabelerView::SharedPtr mLabeler;
        ATProto::AppBskyGraph::StarterPackViewBasic::SharedPtr mStarterPack;
        BlockedAuthor mBlockedAuthor;
        QString mNotFoundUri;
        QString mDetachedByDid;
        QString mDetachedPostUri;
        QString mUnsupportedType;
        QString mContentWarning = "NOT INITIALIZED";
        LanguageList mLanguages;
        QString mFormattedText;
        std::optional<ContentLabelList> mContentLabels;
        std::optional<ContentLabelList> mLabelsIncludingAuthorLabels;
        BasicProfile mContentLabeler;
        QEnums::ContentVisibility mContentVisibility = QEnums::CONTENT_VISIBILITY_HIDE_POST;
        QEnums::MutedPostReason mMutedReason = QEnums::MUTED_POST_NONE;
        bool mNotFound = false;
        bool mBlocked = false;
        bool mDetached = false;
        bool mNotSupported = false;
    };
    std::shared_ptr<PrivateData> mPrivate = std::make_shared<PrivateData>();
};

}

Q_DECLARE_METATYPE(::Skywalker::RecordView)
