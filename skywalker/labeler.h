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
    explicit LabelerViewerState(const ATProto::AppBskyLabeler::LabelerViewerState::SharedPtr& viewerState);

    QString getLike() const;

private:
    ATProto::AppBskyLabeler::LabelerViewerState::SharedPtr mViewerState;
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
    const ContentGroupMap& getLabelContentGroupMap() const { return mLabelContentGroupMap; }

private:
    QStringList mLabelValues;
    ContentGroupMap mLabelContentGroupMap;
    QString mLabelerDid;
};

class LabelerView
{
    Q_GADGET
    Q_PROPERTY(QString uri READ getUri FINAL)
    Q_PROPERTY(QString cid READ getCid FINAL)
    Q_PROPERTY(Profile creator READ getCreator FINAL)
    Q_PROPERTY(int likeCount READ getLikeCount FINAL)
    Q_PROPERTY(LabelerViewerState viewer READ getViewer FINAL)
    Q_PROPERTY(QDateTime indexedAt READ getIndexedAt FINAL)
    Q_PROPERTY(ContentLabelList contentLabels READ getContentLabels FINAL)
    QML_VALUE_TYPE(labelerview)

public:
    LabelerView() = default;
    explicit LabelerView(const ATProto::AppBskyLabeler::LabelerView::SharedPtr& view);
    explicit LabelerView(const ATProto::AppBskyLabeler::LabelerViewDetailed::SharedPtr& view);

    QString getUri() const;
    QString getCid() const;
    Profile getCreator() const;
    int getLikeCount() const;
    LabelerViewerState getViewer() const;
    QDateTime getIndexedAt() const;
    ContentLabelList getContentLabels() const;

protected:
    ATProto::AppBskyLabeler::LabelerViewDetailed::SharedPtr mViewDetailed;

private:
    ATProto::AppBskyLabeler::LabelerView::SharedPtr mView;
};

class LabelerViewDetailed : public LabelerView
{
    Q_GADGET
    Q_PROPERTY(LabelerPolicies policies READ getPolicies FINAL)
    QML_VALUE_TYPE(labelerviewdetailed)

public:
    LabelerViewDetailed() = default;
    explicit LabelerViewDetailed(const ATProto::AppBskyLabeler::LabelerViewDetailed::SharedPtr& view);

    LabelerPolicies getPolicies() const;
};

}

Q_DECLARE_METATYPE(::Skywalker::LabelerViewerState)
Q_DECLARE_METATYPE(::Skywalker::LabelerPolicies)
Q_DECLARE_METATYPE(::Skywalker::LabelerView)
Q_DECLARE_METATYPE(::Skywalker::LabelerViewDetailed)
