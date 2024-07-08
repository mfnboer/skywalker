// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "content_label.h"
#include "enums.h"
#include "image_view.h"
#include "shared_image_provider.h"
#include <atproto/lib/lexicon/app_bsky_graph.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class ListViewerState
{
    Q_GADGET
    Q_PROPERTY(bool muted READ getMuted FINAL)
    Q_PROPERTY(QString blocked READ getBlocked FINAL)
    QML_VALUE_TYPE(listviewerstate)

public:
    ListViewerState() = default;
    explicit ListViewerState(const ATProto::AppBskyGraph::ListViewerState::SharedPtr& viewerState);

    bool getMuted() const;
    QString getBlocked() const;

private:
    ATProto::AppBskyGraph::ListViewerState::SharedPtr mViewerState;
};

class ListViewBasic
{
    Q_GADGET
    Q_PROPERTY(QString uri READ getUri FINAL)
    Q_PROPERTY(QString cid READ getCid FINAL)
    Q_PROPERTY(QString name READ getName FINAL)
    Q_PROPERTY(QEnums::ListPurpose purpose READ getPurpose FINAL)
    Q_PROPERTY(QString avatar READ getAvatar FINAL)
    Q_PROPERTY(ImageView imageView READ getImageView FINAL)
    Q_PROPERTY(ContentLabelList labels READ getContentLabels FINAL)
    Q_PROPERTY(ListViewerState viewer READ getViewer FINAL)
    QML_VALUE_TYPE(listviewbasic)

public:
    ListViewBasic() = default;
    explicit ListViewBasic(const ATProto::AppBskyGraph::ListViewBasic::SharedPtr& view);
    explicit ListViewBasic(const ATProto::AppBskyGraph::ListView::SharedPtr& view);
    ListViewBasic(const QString& uri, const QString& cid, const QString& name,
                  ATProto::AppBskyGraph::ListPurpose purpose, const QString& avatar);

    Q_INVOKABLE bool isNull() const { return getUri().isEmpty(); }
    QString getUri() const;
    QString getCid() const;
    QString getName() const;
    QEnums::ListPurpose getPurpose() const;
    QString getAvatar() const;
    ImageView getImageView() const;
    ContentLabelList getContentLabels() const;
    ListViewerState getViewer() const;

    void setCid(const QString& cid) { mCid = cid; }
    void setName(const QString& name) { mName = name; }

    // If avatar is a "image://", then the list view takes ownership of the image
    void setAvatar(const QString& avatar);

protected:
    ATProto::AppBskyGraph::ListView::SharedPtr mListView;

private:
    ATProto::AppBskyGraph::ListViewBasic::SharedPtr mListViewBasic;

    std::optional<QString> mUri;
    std::optional<QString> mCid;
    std::optional<QString> mName;
    ATProto::AppBskyGraph::ListPurpose mPurpose = ATProto::AppBskyGraph::ListPurpose::UNKNOWN;
    std::optional<QString> mAvatar;
    SharedImageSource::SharedPtr mAvatarSource;
    std::optional<ListViewerState> mViewer;
};

using ListViewBasicList = QList<ListViewBasic>;

}
