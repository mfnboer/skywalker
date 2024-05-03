// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "content_group.h"
#include "profile.h"
#include <atproto/lib/lexicon/app_bsky_labeler.h>
#include <QtQmlIntegration>

namespace Skywalker {

class LabelerViewerState
{
    Q_GADGET
    Q_PROPERTY(QString like READ getLike FINAL)
    QML_VALUE_TYPE(labelerviewerstate)

public:
    LabelerViewerState() = default;
    explicit LabelerViewerState(const ATProto::AppBskyLabeler::LabelerViewerState& viewerState);

    const QString& getLike() const { return mLike; }

private:
    QString mLike;
};

class LabelerPolicies
{
    Q_GADGET
    Q_PROPERTY(QStringList labelValues READ getLabelValues FINAL)
    QML_VALUE_TYPE(labelerpolicies)

public:
    LabelerPolicies() = default;
    explicit LabelerPolicies(const ATProto::AppBskyLabeler::LabelerPolicies& policies, const QString& did);

    const QStringList& getLabelValues() const { return mLabelValues; }
    const ContentGroup* getContentGroup(const QString& label) const;
    std::vector<ContentGroup> getContentGroupList() const;

private:
    QStringList mLabelValues;
    ContentGroupMap mLabelContentGroupMap;
    QString mLabelerDid;
};

class LabelerView
{
    Q_GADGET
    Q_PROPERTY(QString uri READ getUri FINAL)
    Q_PROPERTY(Profile creator READ getCreator FINAL)
    Q_PROPERTY(int likeCount READ getLikeCount FINAL)
    Q_PROPERTY(LabelerViewerState viewer READ getViewer FINAL)
    Q_PROPERTY(QDateTime indexedAt READ getIndexedAt FINAL)
    Q_PROPERTY(ContentLabelList contentLabels READ getContentLabels FINAL)
    QML_VALUE_TYPE(labelerview)

public:
    LabelerView() = default;
    explicit LabelerView(const ATProto::AppBskyLabeler::LabelerView& view);
    explicit LabelerView(const ATProto::AppBskyLabeler::LabelerViewDetailed& view);

    const QString& getUri() const { return mUri; }
    const Profile& getCreator() const { return mCreator; }
    int getLikeCount() const { return mLikeCount; }
    const LabelerViewerState& getViewer() const { return mViewer; }
    const QDateTime& getIndexedAt() const { return mIndexedAt; }
    const ContentLabelList& getContentLabels() const { return mContentLabels; }

private:
    QString mUri;
    // CID is available, but we don't need it now
    Profile mCreator;
    int mLikeCount;
    LabelerViewerState mViewer;
    QDateTime mIndexedAt;
    ContentLabelList mContentLabels;
};

class LabelerViewDetailed : public LabelerView
{
    Q_GADGET
    Q_PROPERTY(LabelerPolicies policies READ getPolicies FINAL)
    QML_VALUE_TYPE(labelerviewdetailed)

public:
    LabelerViewDetailed() = default;
    explicit LabelerViewDetailed(const ATProto::AppBskyLabeler::LabelerViewDetailed& view);

    const LabelerPolicies& getPolicies() const { return mPolicies; }

private:
    LabelerPolicies mPolicies;
};

}

Q_DECLARE_METATYPE(::Skywalker::LabelerViewerState)
Q_DECLARE_METATYPE(::Skywalker::LabelerPolicies)
Q_DECLARE_METATYPE(::Skywalker::LabelerView)
Q_DECLARE_METATYPE(::Skywalker::LabelerViewDetailed)
