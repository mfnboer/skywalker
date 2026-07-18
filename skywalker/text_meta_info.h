// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QQmlEngine>

namespace Skywalker {

class TextMetaInfo
{
    Q_GADGET
    Q_PROPERTY(int newLineCount READ getNewLineCount CONSTANT FINAL)
    Q_PROPERTY(int hasContinuousWhitespace READ hasContinuousWhitespace CONSTANT FINAL)
    Q_PROPERTY(int hasFacets READ hasFacets CONSTANT FINAL)
    Q_PROPERTY(int hasFullHtml READ hasFullHtml CONSTANT FINAL)
    QML_VALUE_TYPE(textmetainfo)

public:
    Q_INVOKABLE bool isNull() const { return mNewLineCount < 0; }
    Q_INVOKABLE bool isSimpleText() const { return !mHasFacets && !mHasContinuousWhitespace; }
    int getNewLineCount() const { return mNewLineCount; }
    bool hasContinuousWhitespace() const { return mHasContinuousWhitespace; }
    bool hasFacets() const { return mHasFacets; }
    bool hasFullHtml() const { return mHasFullHtml; }

    void setNewLineCount(int count) { mNewLineCount = count; }
    void setHasContinousWhitespace(bool continuous) { mHasContinuousWhitespace = continuous; }
    void setHasFacets(bool facets) { mHasFacets = facets; }
    void setHasFullHtml(bool fullHtml) { mHasFullHtml = fullHtml; }

private:
    int mNewLineCount = -1;
    bool mHasContinuousWhitespace = false;
    bool mHasFacets = false;
    bool mHasFullHtml = false;
};

}
