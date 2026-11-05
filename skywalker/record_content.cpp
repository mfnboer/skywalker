// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "record_content.h"

namespace Skywalker {

RecordContent::RecordContent(const QString& text, const QDateTime& createdAt) :
    mText(text),
    mCreatedAt(createdAt)
{
}

}