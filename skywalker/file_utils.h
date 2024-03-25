// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QString>
#include <QDateTime>

namespace Skywalker::FileUtils {

bool checkReadMediaPermission();
bool checkWriteMediaPermission();
QString getAppDataPath(const QString& subDir);
QString getPicturesPath(const QString& subDir);
int openContentUri(const QString& contentUri);
QString resolveContentUriToFile(const QString& contentUri);
QString createDateTimeName(QDateTime timestamp = QDateTime::currentDateTime());

}
