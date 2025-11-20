// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <content_filter.h>
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
    void initTestCase()
    {
        const ContentGroupMap groupMap {
            { "foo", { "foo", "foo title", "foo description", {}, false, QEnums::CONTENT_VISIBILITY_SHOW, QEnums::LABEL_TARGET_CONTENT, QEnums::LABEL_SEVERITY_INFO, FOO_LABELER_DID } }
        };

        mContentFilter.addContentGroupMap(FOO_LABELER_DID, groupMap);
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
        Q_ASSERT(group);
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
        Q_ASSERT(group);
        QCOMPARE(group->getLabelerDid(), FOO_LABELER_DID);
        QCOMPARE(group->getLabelId(), "foo");
        QVERIFY(!group->isAdult());

        group = mContentFilter.getContentGroup("", "foo");
        QVERIFY(!group);
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
};
