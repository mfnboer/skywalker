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

    return unrolledThread;
}

std::pair<int, Post> ThreadUnroller::createThreadPart(const std::deque<Post>& thread, int startIndex)
{
    Q_ASSERT(!thread.empty());
    const Post& firstPost = thread[startIndex];

    if ((int)thread.size() == startIndex + 1)
        return { startIndex + 1, firstPost };

    if (!morePostsCanBeAdded(firstPost))
        return { startIndex + 1, firstPost };

    Post unrolledPost(firstPost);
    QString plainText = unrolledPost.getText();
    QString formattedText = unrolledPost.getFormattedText();

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

    unrolledPost.setOverrideText(plainText);
    unrolledPost.setOverrideFormattedText(formattedText);

    return { postIndex, unrolledPost };
}

bool ThreadUnroller::morePostsCanBeAdded(const Post& post)
{
    return !(post.hasImages() || post.hasVideo() || post.hasExternal() || post.isQuotePost());
}

}
