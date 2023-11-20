// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "image_view.h"
#include "profile.h"
#include <atproto/lib/lexicon/app_bsky_embed.h>
#include <QtQmlIntegration>

namespace Skywalker
{

class RecordView
{
    Q_GADGET
    Q_PROPERTY(QString postUri READ getUri FINAL)
    Q_PROPERTY(QString postTextFormatted READ getFormattedText FINAL)
    Q_PROPERTY(BasicProfile author READ getAuthor FINAL)
    Q_PROPERTY(QDateTime postDateTime READ getIndexedAt FINAL)
    Q_PROPERTY(QList<ImageView> images READ getImages FINAL)
    Q_PROPERTY(QStringList contentLabels READ getLabelTexts FINAL)
    Q_PROPERTY(QEnums::ContentVisibility contentVisibility READ getContentVisibility FINAL)
    Q_PROPERTY(QString contentWarning READ getContentWarning FINAL)
    Q_PROPERTY(QVariant external READ getExternal FINAL)
    Q_PROPERTY(bool notFound READ getNotFound FINAL)
    Q_PROPERTY(bool blocked READ getBlocked FINAL)
    Q_PROPERTY(bool notSupported READ getNotSupported FINAL)
    Q_PROPERTY(QString unsupportedType READ getUnsupportedType FINAL)
    Q_PROPERTY(bool available READ getAvailable FINAL)
    QML_VALUE_TYPE(recordview)

public:
    using Ptr = std::unique_ptr<RecordView>;

    RecordView() = default;
    explicit RecordView(const ATProto::AppBskyEmbed::RecordView& view);

    QString getUri() const;
    QString getFormattedText() const;
    BasicProfile getAuthor() const;
    QDateTime getIndexedAt() const;
    QList<ImageView> getImages() const;
    QVariant getExternal() const;
    QStringList getLabelTexts() const;
    const std::vector<ATProto::ComATProtoLabel::Label::Ptr>& getLabels() const;
    QEnums::ContentVisibility getContentVisibility() const { return mContentVisibility; }
    const QString& getContentWarning() const { return mContentWarning; }

    bool getNotFound() const { return mNotFound; }
    bool getBlocked() const { return mBlocked; }
    bool getNotSupported() const { return mNotSupported; }
    const QString& getUnsupportedType() const { return mUnsupportedType; }
    bool getAvailable() const { return !mNotFound && !mBlocked && !mNotSupported; }

    void setContentVisibility(QEnums::ContentVisibility visibility) { mContentVisibility = visibility; }
    void setContentWarning(const QString& warning) { mContentWarning = warning; }

private:
    const ATProto::AppBskyEmbed::RecordViewRecord* mRecord = nullptr;
    bool mNotFound = false;
    bool mBlocked = false;
    bool mNotSupported = false;
    QString mUnsupportedType;
    QEnums::ContentVisibility mContentVisibility = QEnums::CONTENT_VISIBILITY_HIDE_POST;
    QString mContentWarning = "NOT INITIALIZED";
};

}

Q_DECLARE_METATYPE(Skywalker::RecordView)