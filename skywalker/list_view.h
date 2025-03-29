// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "list_view_include.h"
#include "profile.h"
#include "web_link.h"

namespace Skywalker {

class ListView : public ListViewBasic
{
    Q_GADGET
    Q_PROPERTY(Profile creator READ getCreator FINAL)
    Q_PROPERTY(QString description READ getDescription FINAL)
    Q_PROPERTY(QString formattedDescription READ getFormattedDescription FINAL)
    Q_PROPERTY(WebLink::List embeddedLinksDescription READ getEmbeddedLinksDescription FINAL)
    QML_VALUE_TYPE(listview)

public:
    ListView() = default;
    explicit ListView(const ATProto::AppBskyGraph::ListView::SharedPtr& view);
    ListView(const QString& uri, const QString& cid, const QString& name,
             ATProto::AppBskyGraph::ListPurpose purpose, const QString& avatar,
             const Profile& creator, const QString& description,
             const WebLink::List& embeddedLinks);

    Profile getCreator() const;
    QString getDescription() const;
    QString getFormattedDescription() const;
    WebLink::List getEmbeddedLinksDescription() const;

    void setDescription(const QString& description, const WebLink::List& embeddedLinks);

private:
    std::optional<Profile> mCreator;
    std::optional<QString> mDescription;
    WebLink::List mEmbeddedLinksDescription;
};

}
