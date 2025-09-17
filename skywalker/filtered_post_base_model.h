// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"
#include "post_filter.h"

namespace Skywalker {

class FilteredPostBaseModel : public AbstractPostFeedModel
{
    Q_OBJECT
    Q_PROPERTY(QString feedName READ getFeedName CONSTANT FINAL)
    Q_PROPERTY(QEnums::ContentMode contentMode READ getContentMode CONSTANT FINAL)
    Q_PROPERTY(QColor backgroundColor READ getBackgroundColor CONSTANT FINAL)
    Q_PROPERTY(BasicProfile profile READ getProfile CONSTANT FINAL)
    Q_PROPERTY(QDateTime checkedTillTimestamp READ getCheckedTillTimestamp NOTIFY checkedTillTimestampChanged FINAL)
    Q_PROPERTY(int numPostsChecked READ getNumPostsChecked NOTIFY numPostsCheckedChanged FINAL)

public:
    explicit FilteredPostBaseModel(IPostFilter::Ptr postFilter,
                                   const QString& userDid, const IProfileStore& following,
                                   const IProfileStore& mutedReposts,
                                   const IContentFilter& contentFilter,
                                   const IMatchWords& mutedWords,
                                   const FocusHashtags& focusHashtags,
                                   HashtagIndex& hashtags,
                                   QObject* parent = nullptr);

    Q_INVOKABLE bool isFilterModel() const { return true; }
    QString getFeedName() const { return mPostFilter->getName(); }
    QEnums::ContentMode getContentMode() const { return mPostFilter->getContentMode(); }
    QColor getBackgroundColor() const { return mPostFilter->getBackgroundColor(); }
    BasicProfile getProfile() const { return mPostFilter->getAuthor(); }
    const IPostFilter& getPostFilter() const { return *mPostFilter; }

    void setCheckedTillTimestamp(QDateTime timestamp);
    QDateTime getCheckedTillTimestamp() const { return mCheckedTillTimestamp; }
    void setNumPostsChecked(int numPostsChecked);
    int getNumPostsChecked() const { return mNumPostsChecked; }

signals:
    void checkedTillTimestampChanged();
    void numPostsCheckedChanged();

protected:
    IPostFilter::Ptr mPostFilter;
    QDateTime mCheckedTillTimestamp{QDateTime::currentDateTimeUtc()};
    int mNumPostsChecked = 0;
};

}
