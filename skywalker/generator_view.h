// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "image_view.h"
#include "profile.h"
#include <atproto/lib/lexicon/app_bsky_feed.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class GeneratorViewerState
{
    Q_GADGET
    Q_PROPERTY(QString like READ getLike FINAL)
    QML_VALUE_TYPE(generatorviewerstate)

public:
    GeneratorViewerState() = default;
    explicit GeneratorViewerState(const ATProto::AppBskyFeed::GeneratorViewerState::SharedPtr& viewerState);

    QString getLike() const;

private:
    ATProto::AppBskyFeed::GeneratorViewerState::SharedPtr mViewerState;
};

class GeneratorView
{
    Q_GADGET
    Q_PROPERTY(QString uri READ getUri FINAL)
    Q_PROPERTY(QString cid READ getCid FINAL)
    Q_PROPERTY(Profile creator READ getCreator FINAL)
    Q_PROPERTY(QString displayName READ getDisplayName FINAL)
    Q_PROPERTY(QString description READ getDescription FINAL)
    Q_PROPERTY(QString formattedDescription READ getFormattedDescription FINAL)
    Q_PROPERTY(QString avatar READ getAvatar FINAL)
    Q_PROPERTY(QString avatarThumb READ getAvatarThumb FINAL)
    Q_PROPERTY(ImageView imageView READ getImageView FINAL)
    Q_PROPERTY(int likeCount READ getLikeCount FINAL)
    Q_PROPERTY(bool acceptsInteractions READ acceptsInteractions FINAL)
    Q_PROPERTY(ContentLabelList labels READ getContentLabels FINAL)
    Q_PROPERTY(GeneratorViewerState viewer READ getViewer FINAL)
    QML_VALUE_TYPE(generatorview)

public:
    GeneratorView() = default;
    explicit GeneratorView(const ATProto::AppBskyFeed::GeneratorView::SharedPtr& view);

    QEnums::FavoriteType getFavoriteType() const { return QEnums::FAVORITE_FEED; }

    Q_INVOKABLE bool isNull() const { return mGeneratorView == nullptr; }
    QString getUri() const;
    QString getCid() const;
    QString getDid() const;
    Profile getCreator() const;
    QString getName() const { return getDisplayName(); }
    QString getDisplayName() const;
    QString getDescription() const;
    QString getFormattedDescription() const;
    QString getAvatar() const;
    QString getAvatarThumb() const;
    ImageView getImageView() const;
    int getLikeCount() const;
    bool acceptsInteractions() const;
    ContentLabelList getContentLabels() const;
    GeneratorViewerState getViewer() const;

private:
    ATProto::AppBskyFeed::GeneratorView::SharedPtr mGeneratorView;
};

using GeneratorViewList = QList<GeneratorView>;

}

Q_DECLARE_METATYPE(::Skywalker::GeneratorViewerState)
Q_DECLARE_METATYPE(::Skywalker::GeneratorView)
