// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "constellation.h"
#include "presence.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class Backlinks : public WrappedSkywalker, public Presence
{
public:
    explicit Backlinks(Constellation& constellation, QObject* parent = nullptr);

    void getBlockedByAuthorList(const QString& atId, const QString& cursor, int modelId);

private:
    void getProfiles(const std::vector<QString>& dids, const QString& cursor, int modelId);

    Constellation& mConstellation;
};

}