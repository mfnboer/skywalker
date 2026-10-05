// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "generator_view.h"
#include "named_link.h"
#include "profile.h"
#include <atproto/lib/lexicon/app_bsky_graph.h>

namespace Skywalker {

class StarterPackViewBasic
{
    Q_GADGET
    Q_PROPERTY(int MAX_FEEDS MEMBER MAX_FEEDS CONSTANT)
    Q_PROPERTY(int MAX_MEMBERS MEMBER MAX_MEMBERS CONSTANT)
    Q_PROPERTY(QString uri READ getUri FINAL)
    Q_PROPERTY(QString cid READ getCid FINAL)
    Q_PROPERTY(BasicProfile creator READ getCreator FINAL)
    Q_PROPERTY(QString name READ getName FINAL)
    Q_PROPERTY(QString description READ getDescription FINAL)
    Q_PROPERTY(QString formattedDescription READ getFormattedDescription FINAL)
    Q_PROPERTY(NamedLink::List embeddedLinksDescription READ getEmbeddedLinksDescription FINAL)
    Q_PROPERTY(ContentLabelList labels READ getContentLabels FINAL)
    Q_PROPERTY(int listItemCount READ getListItemCount FINAL)
    QML_VALUE_TYPE(starterpackviewbasic)

public:
    static constexpr int MAX_FEEDS = ATProto::AppBskyGraph::StarterPack::MAX_FEEDS;
    static constexpr int MAX_MEMBERS = 500;

    StarterPackViewBasic() = default;
    explicit StarterPackViewBasic(const ATProto::AppBskyGraph::StarterPackViewBasic::SharedPtr& view);
    explicit StarterPackViewBasic(const ATProto::AppBskyGraph::StarterPackView::SharedPtr& view);
    StarterPackViewBasic(const QString& uri, const QString& cid, const QString& name,
                         const BasicProfile& creator, const QString& description,
                         const NamedLink::List& embeddedLinks);

    Q_INVOKABLE bool isNull() const { return !mBasicView && !mView && !mUri; }
    QString getUri() const;
    QString getCid() const;
    QString getName() const;
    QString getDescription() const;
    QString getFormattedDescription() const;
    NamedLink::List getEmbeddedLinksDescription() const;
    BasicProfile getCreator() const;
    ContentLabelList getContentLabels() const;
    int getListItemCount() const; // -1 = unbknown

    void setCid(const QString& cid) { mCid = cid; }
    void setName(const QString& name) { mName = name; }
    void setDescription(const QString& description, const NamedLink::List& embeddedLinks);

protected:
    ATProto::AppBskyGraph::StarterPackView::SharedPtr mView;

private:
    const ATProto::AppBskyGraph::StarterPack* getStarterPack() const;

    ATProto::AppBskyGraph::StarterPackViewBasic::SharedPtr mBasicView;
    std::optional<QString> mUri;
    std::optional<QString> mCid;
    std::optional<QString> mName;
    std::optional<BasicProfile> mCreator;
    std::optional<QString> mDescription;
    NamedLink::List mEmbeddedLinksDescription;

};

class StarterPackView : public StarterPackViewBasic
{
    Q_GADGET
    Q_PROPERTY(ListViewBasic list READ getList FINAL)
    Q_PROPERTY(GeneratorView::List feeds READ getFeeds FINAL)
    QML_VALUE_TYPE(starterpackview)

public:
    StarterPackView() = default;
    explicit StarterPackView(const ATProto::AppBskyGraph::StarterPackView::SharedPtr& view);
    explicit StarterPackView(const ATProto::AppBskyGraph::StarterPackViewBasic::SharedPtr& view);
    StarterPackView(const QString& uri, const QString& cid, const QString& name,
                    const BasicProfile& creator, const QString& description,
                    const NamedLink::List& embeddedLinks);

    ListViewBasic getList() const;
    GeneratorView::List getFeeds() const;
};

}
