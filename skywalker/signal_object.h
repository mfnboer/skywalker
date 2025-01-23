// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>

namespace Skywalker {

class SignalObject
{
public:
    SignalObject() = default;
    SignalObject(const SignalObject&) = delete;
    SignalObject& operator=(SignalObject&) = delete;

    bool isHandled() const { return mHandled; }
    void setHandled(bool handled) const { mHandled = handled; }

private:
    mutable bool mHandled = false;
};

// When the signal is not handled at destruction, then the file will be removed.
class FileSignal : public SignalObject
{
public:
    using SharedPtr = std::shared_ptr<FileSignal>;

    explicit FileSignal(const QString& fileName);
    ~FileSignal();

    const QString& getFileName() const { return mFileName; }

private:
    QString mFileName;
};

}
