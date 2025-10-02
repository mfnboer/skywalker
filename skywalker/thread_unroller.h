// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "post.h"
#include <deque>

namespace Skywalker {

class ThreadUnroller
{
public:
    static std::deque<Post> unrollThread(const std::deque<Post>& thread);

private:
    static std::pair<int, Post> createThreadPart(const std::deque<Post>& thread, int startIndex);
    static void setThreadStats(const std::deque<Post>& rawThread, std::deque<Post>& unrolledThread);
    static void setThreadTypes(std::deque<Post>& unrolledThread);
    static bool morePostsCanBeAdded(const Post& post);
    static QString getCounter(const QString& text);
    static void removeCounterFromPlainText(QString& text, const QString& counter);
    static void removeCounterFromFormattedText(QString& text, const QString& counter);
};

}
