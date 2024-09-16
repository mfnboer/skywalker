// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QString>
#include <QStringList>

namespace Skywalker {

enum class M3U8StreamType
{
    PLAYLIST,
    VIDEO,
    ERROR
};

struct M3U8Stream
{

};

class M3U8Parser
{
public:
    void clear();
    M3U8StreamType parse(const QByteArray& data);
    const QString& getStream360() const { return mStream360; }
    const QString& getStream720() const { return mStream720; }
    const QStringList& getStreamSegments() const { return mStreamSegments; }
    double getStreamDurationSeconds() const { return mStreamDurationSeconds; }

private:
    bool parseTag(const QString& line);

    QString mStream360;
    QString mStream720;
    QStringList mStreamSegments;
    double mStreamDurationSeconds = 0;
    bool mStartTagSeen = false;
};

}
