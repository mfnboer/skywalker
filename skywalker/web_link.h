// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <qqmlintegration.h>

namespace Skywalker {

class WebLink
{
    Q_GADGET
    Q_PROPERTY(QString link READ getLink FINAL)
    Q_PROPERTY(QString name READ getName FINAL)
    Q_PROPERTY(int startIndex READ getStartIndex FINAL)
    Q_PROPERTY(int endIndex READ getEndIndex FINAL)
    QML_VALUE_TYPE(weblink)

public:
    using List = QList<WebLink>;

    WebLink() = default;
    explicit WebLink(const QString& link, int startIndex, int endIndex, const QString& name = "");

    bool operator==(const WebLink&) const = default;

    const QString& getLink() const { return mLink; }
    const QString& getName() const { return mName; }
    int getStartIndex() const { return mStartIndex; }
    int getEndIndex() const { return mEndIndex; }

    void setName(const QString& name);
    void setStartIndex(int index) { mStartIndex = index; }
    void setEndIndex(int index) { mEndIndex = index; }
    void addToIndexes(int add);

    bool isValidEmbeddedLink() const;
    Q_INVOKABLE bool hasMisleadingName() const { return mHasMisleadingName; }
    Q_INVOKABLE QString getMisleadingNameError() const;

private:
    void checkMisleadingName();

    QString mLink;
    QString mName;
    int mStartIndex = -1;
    int mEndIndex = -1;
    bool mHasMisleadingName = false;
};

}
