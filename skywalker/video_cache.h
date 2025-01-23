// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class VideoHandle : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString link READ getLink FINAL)
    Q_PROPERTY(QString fileName READ getFileName FINAL)

public:
    explicit VideoHandle(QObject* parent = nullptr);
    VideoHandle(const QString& link, const QString& fileName, QObject* parent = nullptr);
    ~VideoHandle();

    Q_INVOKABLE bool isValid() const { return mHandle != nullptr; }
    QString getLink() const { return mHandle ? mHandle->mLink : ""; }
    QString getFileName() const { return mHandle ? mHandle->mFileName : ""; }

private:
    // TODO: no need for pointer anymore
    struct Handle
    {
        ~Handle();

        QString mLink;
        QString mFileName;
    };

    std::shared_ptr<Handle> mHandle;
};


class VideoCache
{
public:
    static VideoCache& instance();

    VideoHandle* putVideo(const QString& link, const QString& fileName);
    VideoHandle* getVideo(const QString& link);
    void unlinkVideo(const QString& link, const QString& fileName);

private:
    static std::unique_ptr<VideoCache> sInstance;

    struct CacheEntry
    {
        QString mFileName;
        int mCount = 1;
    };

    std::unordered_map<QString, CacheEntry> mCache; // link -> entry
};

}
