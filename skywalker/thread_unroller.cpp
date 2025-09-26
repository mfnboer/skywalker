// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "thread_unroller.h"
#include "unicode_fonts.h"
#include <QRegularExpression>

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
    QString plainText = firstPost.getText();
    QString counter = getCounter(plainText);
    removeCounterFromPlainText(plainText, counter);
    QString formattedText = firstPost.getFormattedText();
    removeCounterFromFormattedText(formattedText, counter);

    if ((int)thread.size() == startIndex + 1 || !morePostsCanBeAdded(firstPost))
    {
        Post unrolledPost(firstPost);
        unrolledPost.setOverrideText(plainText);
        unrolledPost.setOverrideFormattedText(formattedText);
        return { startIndex + 1, unrolledPost };
    }

    int postIndex = startIndex + 1;
    for (; postIndex < (int)thread.size(); ++postIndex)
    {
        const auto& post = thread[postIndex];
        QString postText = post.getText();
        counter = getCounter(postText);
        removeCounterFromPlainText(postText, counter);
        QString postFormattedText = post.getFormattedText();
        removeCounterFromFormattedText(postFormattedText, counter);

        const bool newPhrase = UnicodeFonts::hasPhraseEnding(plainText) || UnicodeFonts::hasPhraseStarting(postText);

        plainText += newPhrase ? "\n\n" : " ";
        plainText += postText;
        formattedText += newPhrase ? "<br><br>" : " ";
        formattedText += postFormattedText;

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

    // Could be a reply due to posts getting merged. Make it not a reply, such
    // that the visualizer will offer the user to show older posts of the thread.
    // The thread is completely unrolled.
    unrolledThread.front().setOverrideIsReply(false);

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

static QString matchRegexes(const std::vector<QRegularExpression>& regexes, const QString& text)
{
    for (const auto& re : regexes)
    {
        auto match = re.match(text);

        if (match.hasMatch())
            return match.captured();
    }

    return {};
}

QString ThreadUnroller::getCounter(const QString& text)
{
    static const std::vector<QRegularExpression> counterREs = {
        QRegularExpression{ R"(\( *[0-9]+ */ *[0-9]+ *\) *$)" },
        QRegularExpression{ R"(\[ *[0-9]+ */ *[0-9]+ *\] *$)" },
        QRegularExpression{ R"([0-9]+ */ *[0-9]+ *$)" },
        QRegularExpression{ R"([0-9]+ */ *n *$)" },
        QRegularExpression{ R"([0-9]+ */ *$)" },
        QRegularExpression{ R"(/ *[0-9]+ *$)" }
    };

    QString counter = matchRegexes(counterREs, text);
    qDebug() << "Match counter:" << counter << "text:" << text;

    if (counter.isEmpty())
        return {};

    int end = text.size() - counter.size();
    QTextBoundaryFinder boundaryFinder(QTextBoundaryFinder::Grapheme, text);
    boundaryFinder.setPosition(end);
    int prev = boundaryFinder.toPreviousBoundary();
    QString prefix = "";
    bool emojiAllowed = true;
    QString emoji;

    while (prev >= 0)
    {
        const QString grapheme = text.sliced(prev, end - prev);

        if (emojiAllowed && UnicodeFonts::isEmoji(grapheme))
        {
            prefix = grapheme + prefix;
            emoji = grapheme;
            emojiAllowed = false;
        }
        else if (grapheme == " ")
        {
            prefix = grapheme + prefix;
        }
        else if (grapheme == "\n")
        {
            prefix = grapheme + prefix;
            emojiAllowed = false;
        }
        else
        {
            break;
        }

        end = prev;
        prev = boundaryFinder.toPreviousBoundary();
    }

    // If the counter contains an emoji, we expect white space in front of it
    if (!emoji.isEmpty() && prefix.startsWith(emoji))
        prefix.slice(emoji.size());

    counter = prefix + counter;
    qDebug() << "Prefixed counter:" << counter;
    return counter;
}

void ThreadUnroller::removeCounterFromPlainText(QString& text, const QString& counter)
{
    if (counter.isEmpty())
        return;

    text.slice(0, text.size() - counter.size());
}

void ThreadUnroller::removeCounterFromFormattedText(QString& text, const QString& counter)
{
    if (counter.isEmpty())
        return;

    QString htmlCounter(counter);
    htmlCounter.replace('\n', "<br>");
    int index = text.lastIndexOf(htmlCounter);

    if (index < 0)
    {
        qWarning() << "Cannot find counter:" << htmlCounter << "text:" << text;
        return;
    }

    text.remove(index, htmlCounter.size());
}

}
