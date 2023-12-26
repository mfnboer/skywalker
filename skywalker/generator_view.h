// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
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
    explicit GeneratorViewerState(const ATProto::AppBskyFeed::GeneratorViewerState& viewerState);

    const QString& getLike() const { return mLike; }

private:
    QString mLike;
};

class GeneratorView
{
    Q_GADGET
    Q_PROPERTY(QString uri READ getUri FINAL)
    Q_PROPERTY(QString cid READ getCid FINAL)
    Q_PROPERTY(Profile creator READ getCreator FINAL)
    Q_PROPERTY(QString displayName READ getDisplayName FINAL)
    Q_PROPERTY(QString description READ getDescription FINAL)
    Q_PROPERTY(QString avatar READ getAvatar FINAL)
    Q_PROPERTY(int likeCount READ getLikeCount FINAL)
    Q_PROPERTY(GeneratorViewerState viewer READ getViewer FINAL)
    QML_VALUE_TYPE(generatorview)

public:
    GeneratorView() = default;
    explicit GeneratorView(const ATProto::AppBskyFeed::GeneratorView::SharedPtr& view);

    QString getUri() const;
    QString getCid() const;
    QString getDid() const;
    Profile getCreator() const;
    QString getDisplayName() const;
    QString getDescription() const;
    QString getAvatar() const;
    int getLikeCount() const;
    GeneratorViewerState getViewer() const;

private:
    ATProto::AppBskyFeed::GeneratorView::SharedPtr mGeneratorView;
};

}

Q_DECLARE_METATYPE(Skywalker::GeneratorViewerState)
Q_DECLARE_METATYPE(Skywalker::GeneratorView)
