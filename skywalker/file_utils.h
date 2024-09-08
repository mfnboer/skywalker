// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QString>
#include <QDateTime>
#include <QTemporaryFile>

namespace Skywalker::FileUtils {

bool checkReadMediaPermission();
bool checkWriteMediaPermission();
QString getAppDataPath(const QString& subDir);
QString getPicturesPath(const QString& subDir);
int openContentUri(const QString& contentUri);
QString resolveContentUriToFile(const QString& contentUri);
std::unique_ptr<QTemporaryFile> createTempFile(const QString& fileUri, const QString& fileExtension);
std::unique_ptr<QTemporaryFile> createTempFile(QFile& file, const QString& fileExtension);
std::unique_ptr<QTemporaryFile> createTempFile(int fd, const QString& fileExtension);
QString createDateTimeName(QDateTime timestamp = QDateTime::currentDateTime());

}
