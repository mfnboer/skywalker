// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "thread_unroller.h"
#include "unicode_fonts.h"

namespace Skywalker {

// TODO: strip post counters
// TODO: handle attachments
std::deque<Post> ThreadUnroller::unrollThread(const std::deque<Post>& thread)
{
    if (thread.size() <= 1)
        return thread;

    Post unrolledPost(thread.front());
    unrolledPost.setIsThreadOverride(QEnums::TRIPLE_BOOL_NO);
    QString plainText = unrolledPost.getText();
    QString formattedText = unrolledPost.getFormattedText();

    for (int i = 1; i < (int)thread.size(); ++i)
    {
        const auto& post = thread[i];
        bool phraseEnding = UnicodeFonts::hasPhraseEnding(plainText);
        plainText += phraseEnding ? "\n\n" : " ";
        plainText += post.getText();
        formattedText += phraseEnding ? "<br><br>" : " ";
        formattedText += post.getFormattedText();
    }

    unrolledPost.setOverrideText(plainText);
    unrolledPost.setOverrideFormattedText(formattedText);

    return {unrolledPost};
}

}
