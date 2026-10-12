// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "constellation.h"
#include "enums.h"
#include "presence.h"
#include "wrapped_skywalker.h"
#include <atproto/lib/repo_master.h>

namespace Skywalker {

class Backlinks : public WrappedSkywalker, public Presence
{
public:
    explicit Backlinks(Constellation& constellation, QObject* parent = nullptr);

    void getBlockedByAuthorList(const QString& atId, const QString& cursor, int modelId);
    void getVerifiedByAuthorList(const QString& atId, const QString& cursor, int modelId);
    void getMemberOfListList(const QString& atId, QEnums::ListPurpose purpose, int maxPages, int minEntries, const QString& cursor, int modelId);

private:
    void getAuthorList(const QString& source, const QString& atId, const QString& cursor, int modelId);
    void getProfiles(const std::vector<QString>& dids, const QString& cursor, int modelId);
    void getListsFromListItems(const std::vector<QString>& listItemUris, QEnums::ListPurpose purpose, int maxPages, int minEntries, const QString& cursor, int modelId);
    void getLists(const std::vector<QString>& listUris, QEnums::ListPurpose purpose, int maxPages, int minEntries, const QString& cursor, int modelId);
    ATProto::RepoMaster* repoMaster();

    Constellation& mConstellation;
    std::unique_ptr<ATProto::RepoMaster> mRepoMaster;
};

}