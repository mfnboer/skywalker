// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "m3u8_parser.h"
#include <QStringList>
#include <QDebug>

namespace Skywalker {

void M3U8Parser::clear()
{
    mStream360.clear();
    mStream720.clear();
    mStreamVideo.clear();
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
            continue;

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

            if (!mStreamVideo.isEmpty())
            {
                qDebug() << "Multiple parts";
                return M3U8StreamType::VIDEO_MULTIPART;
            }

            mStreamVideo = line;
        }
    }

    if (!mStreamVideo.isEmpty())
        return M3U8StreamType::VIDEO;

    return M3U8StreamType::PLAYLIST;
}

}
