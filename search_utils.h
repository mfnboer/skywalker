// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "profile.h"
#include "skywalker.h"
#include <QObject>
#include <set>

namespace Skywalker {

class SearchUtils : public QObject, public Presence
{
    Q_OBJECT
    Q_PROPERTY(Skywalker* skywalker READ getSkywalker WRITE setSkywalker NOTIFY skywalkerChanged FINAL REQUIRED)
    Q_PROPERTY(BasicProfileList authorTypeaheadList READ getAuthorTypeaheadList WRITE setAuthorTypeaheadList NOTIFY authorTypeaheadListChanged FINAL)
    QML_ELEMENT

public:
    static std::set<QString> getWords(const QString& text);

    explicit SearchUtils(QObject* parent = nullptr);

    Q_INVOKABLE void searchAuthorsTypeahead(const QString& prefix);

    Skywalker* getSkywalker() const { return mSkywalker; }
    void setSkywalker(Skywalker* skywalker);
    const BasicProfileList& getAuthorTypeaheadList() const { return mAuthorTypeaheadList; }
    void setAuthorTypeaheadList(const BasicProfileList& list);

signals:
    void skywalkerChanged();
    void authorTypeaheadListChanged();

private:
    ATProto::Client* bskyClient();
    void setAuthorTypeaheadList(const ATProto::AppBskyActor::ProfileViewBasicList& profileViewBasicList);

    Skywalker* mSkywalker = nullptr;
    BasicProfileList mAuthorTypeaheadList;
};

}
