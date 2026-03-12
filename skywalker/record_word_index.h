// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "normalized_word_index.h"

namespace Skywalker {

// This class provides all data for a RecordView that is needed by the
// normalized word index. It is put in a separate class to avoid copying
// when the RecordView gets copied by the QML code.
class RecordWordIndex : public NormalizedWordIndex
{
public:
    using Ptr = std::unique_ptr<RecordWordIndex>;

    explicit RecordWordIndex(const ATProto::AppBskyEmbed::RecordViewRecord::SharedPtr& record);

    QString getText() const override;
    BasicProfile getAuthor() const override;
    QList<ImageView> getImages() const override;
    VideoView::Ptr getVideoView() const override;
    ExternalView::Ptr getExternalView() const override;
    std::vector<QString> getHashtags() const override;
    std::vector<QString> getCashtags() const override;
    std::vector<QString> getAllTags() const override;
    std::vector<QString> getWebLinks() const override;

    void setImages(const QList<ImageView>& images) { mImages = images; };
    void setVideo(const VideoView& video) { mVideo = video; }
    void setExternal(const ExternalView& external) { mExternal = external; }

private:
    ATProto::AppBskyEmbed::EmbedView::SharedPtr getEmbedView(ATProto::AppBskyEmbed::EmbedViewType embedViewType) const;

    ATProto::AppBskyEmbed::RecordViewRecord::SharedPtr mRecord;
    QList<ImageView> mImages;
    ExternalView mExternal;
    VideoView mVideo;
};

}
