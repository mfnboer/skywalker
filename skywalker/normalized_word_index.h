// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QHashFunctions>
#include <QString>
#include <unordered_map>
#include <vector>

namespace Skywalker {

class NormalizedWordIndex
{
public:
    virtual ~NormalizedWordIndex() = default;
    virtual QString getText() const = 0;

    const std::vector<QString>& getNormalizedWords() const;
    const std::unordered_map<QString, std::vector<int>>& getUniqueNormalizedWords() const;

private:
    std::vector<QString> mNormalizedWords;

    // normalized word -> indices into mNormalizedWords
    std::unordered_map<QString, std::vector<int>> mUniqueNormalizedWords;
};

}
