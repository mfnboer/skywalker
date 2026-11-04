// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_report_view.h"
#include "convo_ref.h"
#include "convo_message_ref.h"
#include "strong_ref.h"
#include <atproto/lib/at_uri.h>

namespace Skywalker {

OzoneReportView::OzoneReportView(const ATProto::ToolsOzoneInbox::ReportView::SharedPtr& report) :
    mReport(report)
{
}

QString OzoneReportView::getSubjectUri() const
{
    if (!mReport)
        return {};

    if (ATProto::holdsNonNull<ATProto::ComATProtoAdmin::RepoRef::SharedPtr>(mReport->mSubject))
    {
        auto repoRef = std::get<ATProto::ComATProtoAdmin::RepoRef::SharedPtr>(mReport->mSubject);
        return repoRef->mDid;
    }

    if (ATProto::holdsNonNull<ATProto::ComATProtoRepo::StrongRef::SharedPtr>(mReport->mSubject))
    {
        auto strongRef = std::get<ATProto::ComATProtoRepo::StrongRef::SharedPtr>(mReport->mSubject);
        return strongRef->mUri;
    }

    return {};
}

QEnums::OzoneReportSubjectType OzoneReportView::getSubjectType() const
{
    if (!mReport)
        return QEnums::OZONE_REPORT_SUBJECT_UNKNOWN;

    if (ATProto::holdsNonNull<ATProto::ComATProtoAdmin::RepoRef::SharedPtr>(mReport->mSubject))
        return QEnums::OZONE_REPORT_SUBJECT_REPO_REF;

    if (ATProto::holdsNonNull<ATProto::ComATProtoRepo::StrongRef::SharedPtr>(mReport->mSubject))
        return QEnums::OZONE_REPORT_SUBJECT_STRONG_REF;

    if (ATProto::holdsNonNull<ATProto::ChatBskyConvo::ConvoRef::SharedPtr>(mReport->mSubject))
        return QEnums::OZONE_REPORT_SUBJECT_CONVO_REF;

    if (ATProto::holdsNonNull<ATProto::ChatBskyConvo::MessageRef::SharedPtr>(mReport->mSubject))
        return QEnums::OZONE_REPORT_SUBJECT_MESSAGE_REF;

    qWarning() << "Unknown subject type";
    return QEnums::OZONE_REPORT_SUBJECT_UNKNOWN;
}

QVariant OzoneReportView::getSubject() const
{
    switch (getSubjectType())
    {
    case QEnums::OZONE_REPORT_SUBJECT_REPO_REF:
    {
        auto repoRef = std::get<ATProto::ComATProtoAdmin::RepoRef::SharedPtr>(mReport->mSubject);
        return repoRef->mDid;
    }
    case QEnums::OZONE_REPORT_SUBJECT_STRONG_REF:
    {
        auto strongRef = std::get<ATProto::ComATProtoRepo::StrongRef::SharedPtr>(mReport->mSubject);
        return QVariant::fromValue(StrongRef(strongRef));
    }
    case QEnums::OZONE_REPORT_SUBJECT_CONVO_REF:
    {
        auto convoRef = std::get<ATProto::ChatBskyConvo::ConvoRef::SharedPtr>(mReport->mSubject);
        return QVariant::fromValue(ConvoRef(convoRef));
    }
    case QEnums::OZONE_REPORT_SUBJECT_MESSAGE_REF:
    {
        auto messageRef = std::get<ATProto::ChatBskyConvo::MessageRef::SharedPtr>(mReport->mSubject);
        return QVariant::fromValue(ConvoMessageRef(messageRef));
    }
    case QEnums::OZONE_REPORT_SUBJECT_UNKNOWN:
        return {};
    }

    qWarning() << "Unknown subject type";
    return {};
}

QString OzoneReportView::getRecordContent() const
{
    if (!mReport || !mReport->mRecord)
        return {};

    if (ATProto::holdsNonNull<ATProto::AppBskyFeed::Record::Post::SharedPtr>(*mReport->mRecord))
    {
        auto post = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(*mReport->mRecord);
        return post->mText;
    }

    if (ATProto::holdsNonNull<ATProto::AppBskyGraph::List::SharedPtr>(*mReport->mRecord))
    {
        auto list = std::get<ATProto::AppBskyGraph::List::SharedPtr>(*mReport->mRecord);
        return list->mName;
    }

    if (ATProto::holdsNonNull<ATProto::AppBskyFeed::Generator::SharedPtr>(*mReport->mRecord))
    {
        auto feed = std::get<ATProto::AppBskyFeed::Generator::SharedPtr>(*mReport->mRecord);
        return feed->mDisplayName;
    }

    if (ATProto::holdsNonNull<ATProto::AppBskyGraph::StarterPack::SharedPtr>(*mReport->mRecord))
    {
        auto starterPack = std::get<ATProto::AppBskyGraph::StarterPack::SharedPtr>(*mReport->mRecord);
        return starterPack->mName;
    }

    if (ATProto::holdsNonNull<ATProto::AppBskyActor::Status::SharedPtr>(*mReport->mRecord))
    {
        auto status = std::get<ATProto::AppBskyActor::Status::SharedPtr>(*mReport->mRecord);

        if (!status->mEmbed || !ATProto::holdsNonNull<ATProto::AppBskyEmbed::External::SharedPtr>(*status->mEmbed))
            return {};

        auto external = std::get<ATProto::AppBskyEmbed::External::SharedPtr>(*status->mEmbed);
        return external->mExternal->mTitle + "\n" + external->mExternal->mUri;
    }

    if (ATProto::holdsNonNull<ATProto::AppBskyActor::Link::SharedPtr>(*mReport->mRecord))
    {
        auto link = std::get<ATProto::AppBskyActor::Link::SharedPtr>(*mReport->mRecord);

        if (link->mTitle)
            return *link->mTitle + "\n" + link->mUrl;

        return link->mUrl;
    }

    if (ATProto::holdsNonNull<ATProto::UnknownVariant::SharedPtr>(*mReport->mRecord))
    {
        auto unknown = std::get<ATProto::UnknownVariant::SharedPtr>(*mReport->mRecord);
        return unknown->mType;
    }

    return {};
}

QString OzoneReportView::getSubjectDid() const
{
    switch (getSubjectType())
    {
    case QEnums::OZONE_REPORT_SUBJECT_REPO_REF:
    {
        auto repoRef = std::get<ATProto::ComATProtoAdmin::RepoRef::SharedPtr>(mReport->mSubject);
        return repoRef->mDid;
    }
    case QEnums::OZONE_REPORT_SUBJECT_STRONG_REF:
    {
        auto strongRef = std::get<ATProto::ComATProtoRepo::StrongRef::SharedPtr>(mReport->mSubject);
        ATProto::ATUri atUri(strongRef->mUri);
        return atUri.getAuthority();
    }
    case QEnums::OZONE_REPORT_SUBJECT_CONVO_REF:
    {    auto convoRef = std::get<ATProto::ChatBskyConvo::ConvoRef::SharedPtr>(mReport->mSubject);
        return convoRef->mDid;

    }
    case QEnums::OZONE_REPORT_SUBJECT_MESSAGE_REF:
    {
        auto messageRef = std::get<ATProto::ChatBskyConvo::MessageRef::SharedPtr>(mReport->mSubject);
        return messageRef->mDid;
    }
    case QEnums::OZONE_REPORT_SUBJECT_UNKNOWN:
        return {};
    }

    qWarning() << "Unknown subject type";
    return {};
}

}