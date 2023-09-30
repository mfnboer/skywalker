// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <unordered_map>

namespace Skywalker {

template<typename Item>
class ItemStore
{
public:
    int put(Item&& item)
    {
        mItems[mNextItemId] = std::forward<Item>(item);
        return mNextItemId++;
    }

    void remove(int id) { mItems.erase(id); }

    const Item* get(int id) const {
        auto it = mItems.find(id);
        return it != mItems.end() ? &it->second : nullptr;
    }

    void clear() { mItems.clear(); }

    bool empty() const { return mItems.empty(); }
    const std::unordered_map<int, Item>& items() const { return mItems; }

private:
    std::unordered_map<int, Item> mItems;
    int mNextItemId = 1;
};

}
