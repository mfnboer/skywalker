// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QString>

namespace Skywalker::FileUtils {

bool checkReadMediaPermission();
bool checkWriteMediaPermission();
int openContentUri(const QString& contentUri);
QString resolveContentUriToFile(const QString& contentUri);
QString createDateTimeName();

}
