// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "generator_view.h"
#include "image_view.h"
#include "list_view.h"
#include "muted_words.h"
#include "normalized_word_index.h"
#include "profile.h"
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
    Q_PROPERTY(BasicProfile replyToAuthor READ getReplyToAuthor FINAL)
    Q_PROPERTY(QList<ImageView> images READ getImages FINAL)
    Q_PROPERTY(ContentLabelList contentLabels READ getContentLabels FINAL)
    Q_PROPERTY(QEnums::ContentVisibility contentVisibility READ getContentVisibility FINAL)
    Q_PROPERTY(QString contentWarning READ getContentWarning FINAL)
    Q_PROPERTY(QEnums::MutedPostReason mutedReason READ getMutedReason FINAL)
    Q_PROPERTY(QVariant external READ getExternal FINAL)
    Q_PROPERTY(bool notFound READ getNotFound FINAL)
    Q_PROPERTY(bool blocked READ getBlocked FINAL)
    Q_PROPERTY(bool notSupported READ getNotSupported FINAL)
    Q_PROPERTY(QString unsupportedType READ getUnsupportedType FINAL)
    Q_PROPERTY(bool available READ getAvailable FINAL)
    Q_PROPERTY(bool feedAvailable READ getFeedAvailable FINAL)
    Q_PROPERTY(GeneratorView feed READ getFeed FINAL)
    Q_PROPERTY(bool listAvailable READ getListAvailable FINAL)
    Q_PROPERTY(ListView list READ getList FINAL)
    QML_VALUE_TYPE(recordview)

public:
    using Ptr = std::unique_ptr<RecordView>;

    RecordView() = default;
    explicit RecordView(const ATProto::AppBskyEmbed::RecordView& view);

    QString getUri() const;
    QString getCid() const;
    QString getText() const override;
    QString getFormattedText() const;
    BasicProfile getAuthor() const;
    QDateTime getIndexedAt() const;
    QList<ImageView> getImages() const;
    QVariant getExternal() const;
    ContentLabelList getContentLabels() const;
    const std::vector<ATProto::ComATProtoLabel::Label::Ptr>& getLabels() const;
    bool isReply() const;
    BasicProfile getReplyToAuthor() const;
    std::vector<QString> getHashtags() const override;
    QEnums::ContentVisibility getContentVisibility() const { return mContentVisibility; }
    const QString& getContentWarning() const { return mContentWarning; }
    QEnums::MutedPostReason getMutedReason() const { return mMutedReason; }

    bool getNotFound() const { return mNotFound; }
    bool getBlocked() const { return mBlocked; }
    bool getNotSupported() const { return mNotSupported; }
    const QString& getUnsupportedType() const { return mUnsupportedType; }
    bool getAvailable() const { return mRecord != nullptr; }
    bool getFeedAvailable()  const { return mFeed != nullptr; }
    GeneratorView getFeed() const;
    bool getListAvailable() const { return mList != nullptr; }
    ListView getList() const;

    void setContentVisibility(QEnums::ContentVisibility visibility) { mContentVisibility = visibility; }
    void setContentWarning(const QString& warning) { mContentWarning = warning; }
    void setMutedReason(const QEnums::MutedPostReason mutedReason) { mMutedReason = mutedReason; }
    void setMutedReason(const IMutedWords& mutedWords);

private:
    const ATProto::AppBskyEmbed::RecordViewRecord* mRecord = nullptr;
    const ATProto::AppBskyFeed::GeneratorView* mFeed = nullptr;
    const ATProto::AppBskyGraph::ListView* mList = nullptr;
    bool mNotFound = false;
    bool mBlocked = false;
    bool mNotSupported = false;
    QString mUnsupportedType;
    QEnums::ContentVisibility mContentVisibility = QEnums::CONTENT_VISIBILITY_HIDE_POST;
    QString mContentWarning = "NOT INITIALIZED";
    QEnums::MutedPostReason mMutedReason = QEnums::MUTED_POST_NONE;
};

}

Q_DECLARE_METATYPE(::Skywalker::RecordView)
