// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "presence.h"
#include <atproto/lib/rich_text_master.h>
#include <QJsonObject>
#include <QObject>
#include <qqmlintegration.h>

namespace Skywalker {

class NamedLink : public Presence
{
    Q_GADGET
    Q_PROPERTY(QEnums::LinkType linkType READ getLinkType FINAL)
    Q_PROPERTY(QString link READ getLink FINAL)
    Q_PROPERTY(QString name READ getName FINAL)
    Q_PROPERTY(int startIndex READ getStartIndex FINAL)
    Q_PROPERTY(int endIndex READ getEndIndex FINAL)
    QML_VALUE_TYPE(namedlink)

public:
    using List = QList<NamedLink>;
    using SharedPtr = std::shared_ptr<NamedLink>;

    NamedLink() = default;
    NamedLink(const NamedLink&) = default;
    explicit NamedLink(QEnums::LinkType linkType, const QString& link, int startIndex, int endIndex,
                       const QString& name = "");

    bool operator==(const NamedLink&) const = default;
    NamedLink& operator=(const NamedLink&) = default;

    QEnums::LinkType getLinkType() const { return mLinkType; }
    const QString& getLink() const { return mLink; }
    const QString& getName() const { return mName; }
    int getStartIndex() const { return mStartIndex; }
    int getEndIndex() const { return mEndIndex; }

    void setLinkType(QEnums::LinkType linkType) { mLinkType = linkType; }
    void setName(const QString& name);
    void setStartIndex(int index) { mStartIndex = index; }
    void setEndIndex(int index) { mEndIndex = index; }
    void addToIndexes(int add);

    bool isValidEmbeddedLink() const;
    Q_INVOKABLE bool hasMisleadingName() const { return mHasMisleadingName; }
    Q_INVOKABLE QString getMisleadingNameError() const;

    void setTouchingOtherLink(bool touching) { mTouchingOtherLink = touching; }
    Q_INVOKABLE bool isTouchingOtherLink() const { return mTouchingOtherLink; }

    void resolveDidLinkToHandle(const std::function<void()>& doneCb);
    ATProto::RichTextMaster::ParsedMatch toFacet() const;

    QJsonObject toJson() const;

    static SharedPtr fromJson(const QJsonObject& json);
    static std::vector<ATProto::RichTextMaster::ParsedMatch> toFacetList(const List& links);
    static NamedLink fromFacet(const ATProto::RichTextMaster::ParsedMatch& facet);
    static List fromFacetList(const std::vector<ATProto::RichTextMaster::ParsedMatch>& facets);
    static void resolveDidLinksToHandles(List& links, const std::function<void()>& doneCb, int linkIndex = 0);

private:
    void checkMisleadingName();

    QEnums::LinkType mLinkType = QEnums::LINK_TYPE_WEB;
    QString mLink; // for mentions link = @handle (or DID, see Post::getEmbeddedLinks)
    QString mName;
    int mStartIndex = -1;
    int mEndIndex = -1;
    bool mHasMisleadingName = false;
    bool mTouchingOtherLink = false;
};

}
