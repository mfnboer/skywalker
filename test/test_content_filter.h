// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <content_filter.h>
#include <definitions.h>
#include <list_store.h>
#include <profile_store.h>
#include <user_settings.h>
#include <QtTest/QTest>
#include <ranges>

using namespace Skywalker;

static constexpr char const* FOO_LABELER_DID = "did:foo_labeler";

class TestContentFilter : public QObject
{
    Q_OBJECT
private slots:
    void init()
    {
        mFollowing.add(mCamus);

        mUserSettings.setFixedLabelerEnabled(mUserDid, ContentFilter::BLUESKY_MODERATOR_DID, true);

        mUserPreferences.setLabelersPref({ { {FOO_LABELER_DID, {}} }, {} });

        mLabelPrefLists.addList(mListPhilosophers, {}, {});
        mLabelPrefLists.addProfile(mListPhilosophers.getUri(), mKant, "at:item-kant");

        mContentFilter.createFollowingPrefs();
        mContentFilter.createListPref(mListPhilosophers);

        const ContentGroupMap groupMap {
            { "foo", { "foo", "foo title", "foo description", {}, false, QEnums::CONTENT_VISIBILITY_HIDE_POST, QEnums::LABEL_TARGET_CONTENT, QEnums::LABEL_SEVERITY_INFO, FOO_LABELER_DID } },
            { "bar", { "bar", "bar title", "bar description", {}, false, QEnums::CONTENT_VISIBILITY_WARN_POST, QEnums::LABEL_TARGET_CONTENT, QEnums::LABEL_SEVERITY_INFO, FOO_LABELER_DID } }
        };

        mContentFilter.addContentGroupMap(FOO_LABELER_DID, groupMap);
    }

    void cleanup()
    {
        mFollowing.clear();
        mUserSettings.setFixedLabelerEnabled(mUserDid, ContentFilter::BLUESKY_MODERATOR_DID, false);
        mUserPreferences = {};
        mLabelPrefLists.clear();
        mContentFilter.clear();
    }

    void systemLabel()
    {
        QVERIFY(ContentFilter::isSystemLabelId("!hide"));
        QVERIFY(!ContentFilter::isSystemLabelId("hide"));
    }

    void isAdult()
    {
        mUserPreferences.setAdultContent(true);
        QVERIFY(mContentFilter.getAdultContent());

        mUserPreferences.setAdultContent(false);
        QVERIFY(!mContentFilter.getAdultContent());
    }

    void globalContentGroup()
    {
        QVERIFY(ContentFilter::isGlobalLabel("porn"));
        auto* group = mContentFilter.getContentGroup(FOO_LABELER_DID, "porn");
        QVERIFY(group);
        QVERIFY(group->getLabelerDid().isEmpty());
        QCOMPARE(group->getLabelId(), "porn");
        QVERIFY(group->isAdult());

        QVERIFY(!ContentFilter::isGlobalLabel("kiki"));
        group = mContentFilter.getContentGroup(FOO_LABELER_DID, "kiki");
        QVERIFY(!group);
    }

    void labelerContentGroup()
    {
        auto* group = mContentFilter.getContentGroup(FOO_LABELER_DID, "foo");
        QVERIFY(group);
        QCOMPARE(group->getLabelerDid(), FOO_LABELER_DID);
        QCOMPARE(group->getLabelId(), "foo");
        QVERIFY(!group->isAdult());

        group = mContentFilter.getContentGroup("", "foo");
        QVERIFY(!group);
    }

    void groupPrefVisibility() {
        auto* group = mContentFilter.getContentGroup(FOO_LABELER_DID, "foo");
        QVERIFY(group);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group), QEnums::CONTENT_PREF_VISIBILITY_HIDE);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group, FOLLOWING_LIST_URI), QEnums::CONTENT_PREF_VISIBILITY_HIDE);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group, mListPhilosophers.getUri()), QEnums::CONTENT_PREF_VISIBILITY_HIDE);

        mUserPreferences.setLabelVisibility(FOO_LABELER_DID, "foo", ATProto::UserPreferences::LabelVisibility::WARN);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group), QEnums::CONTENT_PREF_VISIBILITY_WARN);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group, FOLLOWING_LIST_URI), QEnums::CONTENT_PREF_VISIBILITY_WARN);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group, mListPhilosophers.getUri()), QEnums::CONTENT_PREF_VISIBILITY_WARN);

        mContentFilter.setListPref(FOLLOWING_LIST_URI, FOO_LABELER_DID, "foo", QEnums::CONTENT_PREF_VISIBILITY_SHOW);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group), QEnums::CONTENT_PREF_VISIBILITY_WARN);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group, FOLLOWING_LIST_URI), QEnums::CONTENT_PREF_VISIBILITY_SHOW);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group, mListPhilosophers.getUri()), QEnums::CONTENT_PREF_VISIBILITY_WARN);

        mContentFilter.setListPref(mListPhilosophers.getUri(), FOO_LABELER_DID, "foo", QEnums::CONTENT_PREF_VISIBILITY_SHOW);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group), QEnums::CONTENT_PREF_VISIBILITY_WARN);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group, FOLLOWING_LIST_URI), QEnums::CONTENT_PREF_VISIBILITY_SHOW);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group, mListPhilosophers.getUri()), QEnums::CONTENT_PREF_VISIBILITY_SHOW);

        mUserPreferences.removeLabelVisibility(FOO_LABELER_DID, "foo");
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group), QEnums::CONTENT_PREF_VISIBILITY_HIDE);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group, FOLLOWING_LIST_URI), QEnums::CONTENT_PREF_VISIBILITY_SHOW);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group, mListPhilosophers.getUri()), QEnums::CONTENT_PREF_VISIBILITY_SHOW);

        mContentFilter.removeListPref(FOLLOWING_LIST_URI, FOO_LABELER_DID, "foo");
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group), QEnums::CONTENT_PREF_VISIBILITY_HIDE);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group, FOLLOWING_LIST_URI), QEnums::CONTENT_PREF_VISIBILITY_HIDE);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group, mListPhilosophers.getUri()), QEnums::CONTENT_PREF_VISIBILITY_SHOW);

        mContentFilter.removeListPref(mListPhilosophers.getUri(), FOO_LABELER_DID, "foo");
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group), QEnums::CONTENT_PREF_VISIBILITY_HIDE);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group, FOLLOWING_LIST_URI), QEnums::CONTENT_PREF_VISIBILITY_HIDE);
        QCOMPARE(mContentFilter.getGroupPrefVisibility(*group, mListPhilosophers.getUri()), QEnums::CONTENT_PREF_VISIBILITY_HIDE);
    }

    void contentLabelVisibility()
    {
        const ContentLabelList labels{ mLabelFoo, mLabelBar };
        mContentFilter.setListPref(FOLLOWING_LIST_URI, FOO_LABELER_DID, "bar", QEnums::CONTENT_PREF_VISIBILITY_SHOW);

        {
            const auto [visibility, warning, index] = mContentFilter.getVisibilityAndWarning(mErnaux.getDid(), labels);
            QCOMPARE(visibility, QEnums::CONTENT_VISIBILITY_HIDE_POST);
            QCOMPARE(warning, "foo title");
            QCOMPARE(index, 0);
        }

        mUserPreferences.setLabelVisibility(FOO_LABELER_DID, "foo", ATProto::UserPreferences::LabelVisibility::SHOW);
        {
            const auto [visibility, warning, index] = mContentFilter.getVisibilityAndWarning(mErnaux.getDid(), labels);
            QCOMPARE(visibility, QEnums::CONTENT_VISIBILITY_WARN_POST);
            QCOMPARE(warning, "bar title");
            QCOMPARE(index, 1);
        }

        {
            const auto [visibility, warning, index] = mContentFilter.getVisibilityAndWarning(mCamus.getDid(), labels);
            QCOMPARE(visibility, QEnums::CONTENT_VISIBILITY_SHOW);
            QCOMPARE(index, -1);
        }

        {
            const auto [visibility, warning, index] = mContentFilter.getVisibilityAndWarning(mKant.getDid(), labels);
            QCOMPARE(visibility, QEnums::CONTENT_VISIBILITY_WARN_POST);
            QCOMPARE(warning, "bar title");
            QCOMPARE(index, 1);
        }

        mContentFilter.setListPref(mListPhilosophers.getUri(), FOO_LABELER_DID, "bar", QEnums::CONTENT_PREF_VISIBILITY_SHOW);

        {
            const auto [visibility, warning, index] = mContentFilter.getVisibilityAndWarning(mCamus.getDid(), labels);
            QCOMPARE(visibility, QEnums::CONTENT_VISIBILITY_SHOW);
            QCOMPARE(index, -1);
        }
    }

    void conflictingPrefs()
    {
        const ContentLabelList labels{ mLabelFoo, mLabelBar };
        mFollowing.add(mKant);
        mUserPreferences.setLabelVisibility(FOO_LABELER_DID, "foo", ATProto::UserPreferences::LabelVisibility::SHOW);
        mContentFilter.setListPref(FOLLOWING_LIST_URI, FOO_LABELER_DID, "bar", QEnums::CONTENT_PREF_VISIBILITY_WARN);
        mContentFilter.setListPref(mListPhilosophers.getUri(), FOO_LABELER_DID, "bar", QEnums::CONTENT_PREF_VISIBILITY_SHOW);

        const auto [visibility, warning, index] = mContentFilter.getVisibilityAndWarning(mKant.getDid(), labels);
        QCOMPARE(visibility, QEnums::CONTENT_VISIBILITY_SHOW);
        QCOMPARE(index, -1);
    }

    void labelerSubscription()
    {
        QVERIFY(mContentFilter.isSubscribedToLabeler(ContentFilter::BLUESKY_MODERATOR_DID));
        QVERIFY(mContentFilter.isSubscribedToLabeler(FOO_LABELER_DID));
        QVERIFY(!mContentFilter.isSubscribedToLabeler("did:xyz"));
    }

    void fixedLabeler()
    {
        QVERIFY(mContentFilter.isFixedLabelerEnabled(ContentFilter::BLUESKY_MODERATOR_DID));
        QVERIFY(mContentFilter.isSubscribedToLabeler(ContentFilter::BLUESKY_MODERATOR_DID));

        mContentFilter.enableFixedLabeler(ContentFilter::BLUESKY_MODERATOR_DID, false);

        QVERIFY(!mContentFilter.isFixedLabelerEnabled(ContentFilter::BLUESKY_MODERATOR_DID));
        QVERIFY(mContentFilter.isSubscribedToLabeler(ContentFilter::BLUESKY_MODERATOR_DID));

        mContentFilter.enableFixedLabeler(ContentFilter::BLUESKY_MODERATOR_DID, true);

        QVERIFY(mContentFilter.isFixedLabelerEnabled(ContentFilter::BLUESKY_MODERATOR_DID));
        QVERIFY(mContentFilter.isSubscribedToLabeler(ContentFilter::BLUESKY_MODERATOR_DID));
    }

    void followingPrefs()
    {
        QVERIFY(mContentFilter.hasFollowingPrefs());
        mContentFilter.removeFollowing();
        QVERIFY(!mContentFilter.hasFollowingPrefs());
        mContentFilter.createFollowingPrefs();
        QVERIFY(mContentFilter.hasFollowingPrefs());
    }

    void listPrefs()
    {
        QVERIFY(!mContentFilter.hasListPref(mListPhilosophers.getUri(), FOO_LABELER_DID));
        mContentFilter.setListPref(mListPhilosophers.getUri(), FOO_LABELER_DID, "bar", QEnums::CONTENT_PREF_VISIBILITY_SHOW);
        QVERIFY(mContentFilter.hasListPref(mListPhilosophers.getUri(), FOO_LABELER_DID));
        mContentFilter.removeList(mListPhilosophers.getUri());
        QVERIFY(!mContentFilter.hasListPref(mListPhilosophers.getUri(), FOO_LABELER_DID));
        mContentFilter.createListPref(mListPhilosophers);
        mContentFilter.setListPref(mListPhilosophers.getUri(), FOO_LABELER_DID, "bar", QEnums::CONTENT_PREF_VISIBILITY_SHOW);
        QVERIFY(mContentFilter.hasListPref(mListPhilosophers.getUri(), FOO_LABELER_DID));
    }

    void unknownLabel()
    {
        ContentLabel labelFoobar{ FOO_LABELER_DID, "at:foobar", "cid-foobar", "foobar", {} };
        const ContentLabelList labels{ labelFoobar };
        const auto [visibility, warning, index] = mContentFilter.getVisibilityAndWarning(mKant.getDid(), labels);
        QCOMPARE(visibility, QEnums::CONTENT_VISIBILITY_SHOW);
    }

    void initUserSettings()
    {
        const ContentLabelList labels{ mLabelFoo };

        {
            const auto [visibility, warning, index] = mContentFilter.getVisibilityAndWarning(mCamus.getDid(), labels);
            QCOMPARE(visibility, QEnums::CONTENT_VISIBILITY_HIDE_POST);
            QCOMPARE(warning, "foo title");
        }

        {
            const auto [visibility, warning, index] = mContentFilter.getVisibilityAndWarning(mKant.getDid(), labels);
            QCOMPARE(visibility, QEnums::CONTENT_VISIBILITY_HIDE_POST);
            QCOMPARE(warning, "foo title");
        }

        mUserSettings.setContentLabelPref(mUserDid, FOLLOWING_LIST_URI, FOO_LABELER_DID, "foo", QEnums::CONTENT_PREF_VISIBILITY_WARN);
        mUserSettings.setContentLabelPref(mUserDid, mListPhilosophers.getUri(), FOO_LABELER_DID, "foo", QEnums::CONTENT_PREF_VISIBILITY_SHOW);
        mContentFilter.initListPrefs();

        {
            const auto [visibility, warning, index] = mContentFilter.getVisibilityAndWarning(mCamus.getDid(), labels);
            QCOMPARE(visibility, QEnums::CONTENT_VISIBILITY_WARN_POST);
            QCOMPARE(warning, "foo title");
        }

        {
            const auto [visibility, warning, index] = mContentFilter.getVisibilityAndWarning(mKant.getDid(), labels);
            QCOMPARE(visibility, QEnums::CONTENT_VISIBILITY_SHOW);
        }
    }

private:
    class MockUserSettings : public IUserSettingsContentFilter
    {
    public:
        void setContentLabelPref(
            const QString&, const QString& listUri, const QString& labelerDid,
            const QString& labelId, QEnums::ContentPrefVisibility pref) override
        {
            mListLabelPrefs[listUri][labelerDid][labelId] = pref;
        }

        QEnums::ContentPrefVisibility getContentLabelPref(
            const QString&, const QString& listUri,
            const QString& labelerDid, const QString& labelId) const override
        {
            const auto listIt = mListLabelPrefs.find(listUri);

            if (listIt == mListLabelPrefs.end())
                return QEnums::CONTENT_PREF_VISIBILITY_SHOW;

            const auto labelerIt = listIt->second.find(labelerDid);

            if (labelerIt == listIt->second.end())
                return QEnums::CONTENT_PREF_VISIBILITY_SHOW;

            const auto labelIt = labelerIt->second.find(labelId);

            if (labelIt == labelerIt->second.end())
                return QEnums::CONTENT_PREF_VISIBILITY_SHOW;

            return labelIt->second;
        }

        void removeContentLabelPref(const QString&, const QString& listUri, const QString& labelerDid,
                                            const QString& labelId) override
        {
            mListLabelPrefs[listUri][labelerDid].erase(labelId);
        }


        void removeContentLabelPrefList(const QString&, QString listUri) override
        {
            mListLabelPrefs.erase(listUri);
        }

        // tuple: listUri, labelerDid, labelId
        std::vector<std::tuple<QString, QString, QString>> getContentLabelPrefKeys(const QString&) const override

        {
            std::vector<std::tuple<QString, QString, QString>> result;

            for (const auto& [listUri, labelerPrefs] : mListLabelPrefs)
            {
                for (const auto& [labelerDid, labelPrefs] : labelerPrefs)
                {
                    for (const auto& [labelId, _] : labelPrefs)
                        result.push_back({ listUri, labelerDid, labelId} );
                }
            }

            return result;
        }

        QStringList getContentLabelPrefListUris(const QString&) const override
        {
            const auto listUris = mListLabelPrefs | std::ranges::views::keys;
            return { listUris.begin(), listUris.end() };
        }

        QStringList getLabels(const QString&, const QString& labelerDid) const override
        {
            const auto it = mLabelerLabels.find(labelerDid);
            return it != mLabelerLabels.end() ? it->second : QStringList{};
        }

        void setLabels(const QString&, const QString& labelerDid, const QStringList labels) override
        {
            mLabelerLabels[labelerDid] = labels;
        }

        void removeLabels(const QString&, const QString& labelerDid) override
        {
            mLabelerLabels.erase(labelerDid);
        }

        bool containsLabeler(const QString&, const QString& labelerDid) const override
        {
            return mLabelerLabels.contains(labelerDid);
        }

        bool getFixedLabelerEnabled(const QString&, const QString& labelerDid) const override
        {
            return mFixedLabelerEnabled.contains(labelerDid) && mFixedLabelerEnabled.at(labelerDid);
        }

        void setFixedLabelerEnabled(const QString&, const QString& labelerDid, bool enabled) override
        {
            mFixedLabelerEnabled[labelerDid] = enabled;
        }

    private:
        std::unordered_map<QString, std::unordered_map<QString , std::unordered_map<QString, QEnums::ContentPrefVisibility>>> mListLabelPrefs;
        std::unordered_map<QString, QStringList> mLabelerLabels;
        std::unordered_map<QString, bool> mFixedLabelerEnabled;
    };

    class MockListStore : public ListStore
    {
    public:
        void loadList(const QString&, const SuccessCb& successCb, const ErrorCb&,
                      int = 2, int = 0, const QString& = {}) override
        {
            if (successCb)
                successCb();
        }
    };

    QString mUserDid = "did:test";
    MockUserSettings mUserSettings;
    ATProto::UserPreferences mUserPreferences;
    ProfileStore mFollowing;
    MockListStore mLabelPrefLists;
    ContentFilter mContentFilter{ mUserDid, mFollowing, mLabelPrefLists, mUserPreferences, &mUserSettings, this };

    ListViewBasic mListPhilosophers{ "at:philosophers", "cid-philosophers", "Philosophers", ATProto::AppBskyGraph::ListPurpose::CURATE_LIST, {} };
    BasicProfile mKant{ "did:kant", "kant.koningbergen.de", "Immanuel Kant", "" };

    BasicProfile mCamus{ "did:camus", "camus.stranger.fr", "Albert Camus", "" };
    BasicProfile mErnaux{ "did:ernaux", "ernaux.annees.fr", "Annie Ernaux", "" };

    ContentLabel mLabelFoo{ FOO_LABELER_DID, "at:foo", "cid-foo", "foo", {} };
    ContentLabel mLabelBar{ FOO_LABELER_DID, "at:bar", "cid-bar", "bar", {} };
};
