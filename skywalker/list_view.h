// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "profile.h"
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
    explicit ListViewerState(const ATProto::AppBskyGraph::ListViewerState& viewerState);

    bool getMuted() const { return mMuted; }
    const QString& getBlocked() const { return mBlocked; }

private:
    bool mMuted = false;
    QString mBlocked;
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
    Q_PROPERTY(ListViewerState viewer READ getViewer FINAL)
    QML_VALUE_TYPE(listviewbasic)

public:
    ListViewBasic() = default;
    explicit ListViewBasic(const ATProto::AppBskyGraph::ListViewBasic::SharedPtr& view);
    explicit ListViewBasic(const ATProto::AppBskyGraph::ListViewBasic* view);
    explicit ListViewBasic(const ATProto::AppBskyGraph::ListView* view);

    Q_INVOKABLE bool isNull() const { return basicView() == nullptr && view() == nullptr; }
    QString getUri() const;
    QString getCid() const;
    QString getName() const;
    QEnums::ListPurpose getPurpose() const;
    QString getAvatar() const;
    ImageView getImageView() const;
    ListViewerState getViewer() const;

    void setCid(const QString& cid) { mCid = cid; }
    void setName(const QString& name) { mName = name; }
    void setAvatar(const QString& avatar);

protected:
    const ATProto::AppBskyGraph::ListView* view() const;

    const ATProto::AppBskyGraph::ListView* mRawListView = nullptr;

private:
    const ATProto::AppBskyGraph::ListViewBasic* basicView() const;

    ATProto::AppBskyGraph::ListViewBasic::SharedPtr mSharedListViewBasic;
    const ATProto::AppBskyGraph::ListViewBasic* mRawListViewBasic = nullptr;

    QString mCid;
    QString mName;
    std::optional<QString> mAvatar;
    SharedImageSource::SharedPtr mAvatarSource;
};

class ListView : public ListViewBasic
{
    Q_GADGET
    Q_PROPERTY(Profile creator READ getCreator FINAL)
    Q_PROPERTY(QString description READ getDescription FINAL)
    Q_PROPERTY(QString formattedDescription READ getFormattedDescription FINAL)
    QML_VALUE_TYPE(listview)

public:
    ListView() = default;
    explicit ListView(const ATProto::AppBskyGraph::ListView::SharedPtr& view);
    explicit ListView(const ATProto::AppBskyGraph::ListView* view);

    Profile getCreator() const;
    QString getDescription() const;
    QString getFormattedDescription() const;

    void setDescription(const QString& description) { mDescription = description; };

private:
    ATProto::AppBskyGraph::ListView::SharedPtr mSharedListView;
    std::optional<QString> mDescription;
};

}
