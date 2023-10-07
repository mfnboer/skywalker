// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/com_atproto_label.h>
#include <QStringList>

namespace Skywalker {

class ContentFilter {
public:
    using LabelList = std::vector<ATProto::ComATProtoLabel::Label::Ptr>;

    explicit ContentFilter(const LabelList& labels);

    QStringList getLabelTexts() const;

private:
    const LabelList& mLabels;
};

}
