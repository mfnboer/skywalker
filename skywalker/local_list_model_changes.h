// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QHashFunctions>
#include <QString>
#include <optional>
#include <unordered_map>

namespace Skywalker {

class  LocalListModelChanges
{
public:
    struct Change
    {
        // Not-set means not changed.
        // Empty means block removed.
        std::optional<QString> mBlocked;

        std::optional<bool> mMuted;
    };

    LocalListModelChanges() = default;
    virtual ~LocalListModelChanges() = default;

    const Change* getLocalChange(const QString& uri) const;
    void clearLocalChanges();

    void updateBlocked(const QString& uri, const QString& blockUri);
    void updateMuted(const QString& uri, bool muted);

protected:
    virtual void blockedChanged() = 0;
    virtual void mutedChanged() = 0;

private:
    // URI to change
    std::unordered_map<QString, Change> mChanges;
};

}
