// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "m3u8_parser.h"
#include <QDebug>

namespace Skywalker {

void M3U8Parser::clear()
{
    mStream360.clear();
    mStream720.clear();
    mStreamSegments.clear();
    mStreamDurationSeconds = 0;
    mStartTagSeen = false;
}

// Really sparse parser to strip streams from the bsky video server.
// Needs more work.
M3U8StreamType M3U8Parser::parse(const QByteArray& data)
{
    clear();

    const QString text = QString::fromUtf8(data);
    const QStringList lines = text.split('\n');

    for (const auto& l : lines)
    {
        const auto line = l.trimmed();
        qDebug() << line;

        if (line.isEmpty())
            continue;

        if (line.startsWith('#'))
        {
            if (!parseTag(line))
                return M3U8StreamType::ERROR;

            continue;
        }

        if (line.contains(".m3u8"))
        {
            qDebug() << "Playlist:" << line;

            if (line.startsWith("360p"))
                mStream360 = line;
            else
                mStream720 = line;
        }
        else
        {
            qDebug() << "Video stream:" << line;
            mStreamSegments.push_back(line);
        }
    }

    if (!mStreamSegments.isEmpty())
        return M3U8StreamType::VIDEO;

    return M3U8StreamType::PLAYLIST;
}

bool M3U8Parser::parseTag(const QString& line)
{
    if (!line.startsWith("#EXT"))
        return true;

    if (!mStartTagSeen)
    {
        if (line == "#EXTM3U")
        {
            qDebug() << "Start tag";
            mStartTagSeen = true;
            return true;
        }
        else
        {
            qWarning() << "Not a valid playlist";
            return false;
        }
    }

    if (line.startsWith("#EXTINF:"))
    {
        const QString tagData = line.sliced(8);
        const QString durationString = tagData.split(',').first();
        const double duration = durationString.toDouble();
        mStreamDurationSeconds += duration;
        qDebug() << "Duration:" << duration << "total:" << mStreamDurationSeconds;
        return true;
    }

    return true;
}

}
