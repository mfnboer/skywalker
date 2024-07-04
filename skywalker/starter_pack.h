// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "generator_view.h"
#include "profile.h"
#include <atproto/lib/lexicon/app_bsky_graph.h>

namespace Skywalker {

class StarterPackViewBasic
{
    Q_GADGET
    Q_PROPERTY(QString uri READ getUri FINAL)
    Q_PROPERTY(QString cid READ getCid FINAL)
    Q_PROPERTY(BasicProfile creator READ getCreator FINAL)
    Q_PROPERTY(QString name READ getName FINAL)
    Q_PROPERTY(QString description READ getDescription FINAL)
    Q_PROPERTY(QString formattedDescription READ getFormattedDescription FINAL)
    Q_PROPERTY(ContentLabelList labels READ getContentLabels FINAL)
    QML_VALUE_TYPE(starterpackviewbasic)

public:
    StarterPackViewBasic() = default;
    explicit StarterPackViewBasic(const ATProto::AppBskyGraph::StarterPackViewBasic::SharedPtr& view);
    explicit StarterPackViewBasic(const ATProto::AppBskyGraph::StarterPackView::SharedPtr& view);

    Q_INVOKABLE bool isNull() const { return !mBasicView && !mView; }
    QString getUri() const;
    QString getCid() const;
    QString getName() const;
    QString getDescription() const;
    QString getFormattedDescription() const;
    BasicProfile getCreator() const;
    ContentLabelList getContentLabels() const;

protected:
    ATProto::AppBskyGraph::StarterPackView::SharedPtr mView;

private:
    const ATProto::AppBskyGraph::StarterPack* getStarterPack() const;

    ATProto::AppBskyGraph::StarterPackViewBasic::SharedPtr mBasicView;
};

class StarterPackView : public StarterPackViewBasic
{
    Q_GADGET
    Q_PROPERTY(ListViewBasic list READ getList FINAL)
    Q_PROPERTY(GeneratorViewList feeds READ getFeeds FINAL)
    QML_VALUE_TYPE(starterpackview)

public:
    StarterPackView() = default;
    explicit StarterPackView(const ATProto::AppBskyGraph::StarterPackView::SharedPtr& view);

    ListViewBasic getList() const;
    GeneratorViewList getFeeds() const;
};

}
