// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>

namespace Skywalker {

class VideoHandle : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString link READ getLink CONSTANT FINAL)
    Q_PROPERTY(QString fileName READ getFileName CONSTANT FINAL)

public:
    explicit VideoHandle(QObject* parent = nullptr);
    VideoHandle(const QString& link, const QString& fileName, QObject* parent = nullptr);
    ~VideoHandle();

    Q_INVOKABLE bool isValid() const { return !mLink.isEmpty() && !mFileName.isEmpty(); }
    QString getLink() const { return mLink; }
    QString getFileName() const { return mFileName; }

private:
    QString mLink;
    QString mFileName;
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
