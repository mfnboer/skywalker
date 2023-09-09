// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
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
    Q_PROPERTY(QString postText READ getText FINAL)
    Q_PROPERTY(BasicProfile author READ getAuthor FINAL)
    Q_PROPERTY(QDateTime postDateTime READ getIndexedAt FINAL)
    Q_PROPERTY(QList<ImageView> images READ getImages FINAL)
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
    QString getText() const;
    BasicProfile getAuthor() const;
    QDateTime getIndexedAt() const;
    QList<ImageView> getImages() const;
    QVariant getExternal() const;

    bool getNotFound() const { return mNotFound; }
    bool getBlocked() const { return mBlocked; }
    bool getNotSupported() const { return mNotSupported; }
    const QString& getUnsupportedType() const { return mUnsupportedType; }
    bool getAvailable() const { return !mNotFound && !mBlocked && !mNotSupported; }

private:
    const ATProto::AppBskyEmbed::RecordViewRecord* mRecord = nullptr;
    bool mNotFound = false;
    bool mBlocked = false;
    bool mNotSupported = false;
    QString mUnsupportedType;
};

}

Q_DECLARE_METATYPE(Skywalker::RecordView)
