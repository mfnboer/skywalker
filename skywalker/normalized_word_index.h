// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QHashFunctions>
#include <QString>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Skywalker {

class NormalizedWordIndex
{
public:
    virtual ~NormalizedWordIndex() = default;
    virtual QString getText() const = 0;
    virtual std::vector<QString> getHashtags() const = 0;

    const std::unordered_set<QString>& getUniqueHashtags() const;
    const std::vector<QString>& getNormalizedWords() const;
    const std::unordered_map<QString, std::vector<int>>& getUniqueNormalizedWords() const;

private:
    std::unordered_set<QString> mHashtags;
    std::vector<QString> mNormalizedWords;

    // normalized word -> indices into mNormalizedWords
    std::unordered_map<QString, std::vector<int>> mUniqueNormalizedWords;
};

}
