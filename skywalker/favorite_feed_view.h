// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "generator_view.h"
#include "list_view.h"

namespace Skywalker {

class FavoriteFeedView
{
    Q_GADGET
    Q_PROPERTY(bool isGeneratorView READ isGeneratorView FINAL)
    Q_PROPERTY(QString uri READ getUri FINAL)
    Q_PROPERTY(QString name READ getName FINAL)
    Q_PROPERTY(QString avatar READ getAvatar FINAL)
    Q_PROPERTY(GeneratorView generatorView READ getGeneratorView FINAL)
    Q_PROPERTY(ListView listView READ getListView FINAL)
    QML_VALUE_TYPE(favoritefeedview)

public:
    FavoriteFeedView() = default;
    explicit FavoriteFeedView(const GeneratorView& generatorView);
    explicit FavoriteFeedView(const ListView& listView);

    bool isGeneratorView() const { return !mGeneratorView.isNull(); }
    QString getUri() const;
    QString getName() const;
    QString getAvatar() const;
    const GeneratorView& getGeneratorView() const { return mGeneratorView; }
    const ListView& getListView() const { return mListView; }

private:
    GeneratorView mGeneratorView;
    ListView mListView;
};

}

Q_DECLARE_METATYPE(Skywalker::FavoriteFeedView)
