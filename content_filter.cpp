// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "content_filter.h"

namespace Skywalker {

ContentFilter::ContentFilter(const LabelList& labels) :
    mLabels(labels)
{
}

QStringList ContentFilter::getLabelTexts() const
{
    QStringList labelTexts;

    for (const auto& label : mLabels)
    {
        if (!label->mNeg)
            labelTexts.append(label->mVal);
        else
            labelTexts.removeAll(label->mVal);
    }

    return labelTexts;
}

}
