// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "thread_unroller.h"
#include "unicode_fonts.h"

namespace Skywalker {

// TODO: strip post counters
std::deque<Post> ThreadUnroller::unrollThread(const std::deque<Post>& thread)
{
    if (thread.size() <= 1)
        return thread;

    int index = 0;
    std::deque<Post> unrolledThread;

    while (index < (int)thread.size())
    {
        const auto [nextIndex, threadPart] = createThreadPart(thread, index);
        unrolledThread.push_back(threadPart);
        index = nextIndex;
    }

    setThreadStats(thread, unrolledThread);
    setThreadTypes(unrolledThread);
    return unrolledThread;
}

std::pair<int, Post> ThreadUnroller::createThreadPart(const std::deque<Post>& thread, int startIndex)
{
    Q_ASSERT(!thread.empty());
    Q_ASSERT(startIndex >= 0);
    Q_ASSERT(startIndex < (int)thread.size());

    const Post& firstPost = thread[startIndex];

    if ((int)thread.size() == startIndex + 1)
        return { startIndex + 1, firstPost };

    if (!morePostsCanBeAdded(firstPost))
        return { startIndex + 1, firstPost };

    QString plainText = firstPost.getText();
    QString formattedText = firstPost.getFormattedText();

    int postIndex = startIndex + 1;
    for (; postIndex < (int)thread.size(); ++postIndex)
    {
        const auto& post = thread[postIndex];
        bool phraseEnding = UnicodeFonts::hasPhraseEnding(plainText);
        plainText += phraseEnding ? "\n\n" : " ";
        plainText += post.getText();
        formattedText += phraseEnding ? "<br><br>" : " ";
        formattedText += post.getFormattedText();

        if (!morePostsCanBeAdded(post))
        {
            ++postIndex;
            break;
        }
    }

    Post unrolledPost(thread[postIndex - 1]);
    unrolledPost.setOverrideText(plainText);
    unrolledPost.setOverrideFormattedText(formattedText);

    return { postIndex, unrolledPost };
}

// For the last post in the unrolled thread, the stats are shown.
// Aggregate/copy stats from previous posts.
void ThreadUnroller::setThreadStats(const std::deque<Post>& rawThread, std::deque<Post>& unrolledThread)
{
    Q_ASSERT(!rawThread.empty());
    Q_ASSERT(!unrolledThread.empty());

    if (rawThread.size() <= 1)
        return;

    const auto& firstPost = rawThread.front();
    auto& lastPost = unrolledThread.back();

    // Set URI/CID from first post, such the bookmarking in quoting
    // will rever to the first post in the thread.
    lastPost.setOverrideUri(firstPost.getUri());
    lastPost.setOverrideCid(firstPost.getCid());

    lastPost.setOverrideIndexedAt(firstPost.getIndexedAt());
    lastPost.setOverrideLikeUri(firstPost.getLikeUri());
    lastPost.setOverrideRepostUri(firstPost.getRepostUri());
    lastPost.setOverrideBookmarked(firstPost.isBookmarked());
    lastPost.setOverrideThreadMuted(firstPost.isThreadMuted());
    lastPost.setOverrideEmbeddingDisabled(firstPost.isEmbeddingDisabled());
    lastPost.setOverrideReplyDisabled(firstPost.isReplyDisabled());

    int replyCount = 0;
    int repostCount = 0;
    int likeCount = 0;
    int quoteCount = 0;

    for (const auto& post : rawThread)
    {
        replyCount += post.getReplyCount();
        repostCount += post.getRepostCount();
        likeCount += post.getLikeCount();
        quoteCount += post.getQuoteCount();
    }

    // Subtract own replies due to the thread posts
    lastPost.setOverrideReplyCount(replyCount - (rawThread.size() - 1));

    lastPost.setOverrideRepostCount(repostCount);
    lastPost.setOverrideLikeCount(likeCount);
    lastPost.setOverrideQuoteCount(quoteCount);
}

void ThreadUnroller::setThreadTypes(std::deque<Post>& unrolledThread)
{
    if (unrolledThread.empty())
        return;

    for (auto& post : unrolledThread)
        post.setThreadType(QEnums::THREAD_NONE);

    unrolledThread.front().addThreadType(QEnums::THREAD_TOP);
    unrolledThread.back().addThreadType(QEnums::THREAD_LEAF);

    if (unrolledThread.size() < 2)
        return;

    unrolledThread.front().addThreadType(QEnums::THREAD_PARENT);
    unrolledThread[1].addThreadType(QEnums::THREAD_FIRST_DIRECT_CHILD);
    unrolledThread[1].addThreadType(QEnums::THREAD_DIRECT_CHILD);

    for (int i = 1; i < (int)unrolledThread.size(); ++i)
        unrolledThread[i].addThreadType(QEnums::THREAD_CHILD);
}

bool ThreadUnroller::morePostsCanBeAdded(const Post& post)
{
    return !(post.hasImages() || post.hasVideo() || post.hasExternal() || post.isQuotePost());
}

}
