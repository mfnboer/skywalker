// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "image_utils.h"
#include "jni_callback.h"
#include "photo_picker.h"

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

namespace Skywalker {

ImageUtils::ImageUtils(QObject* parent) :
    QObject(parent)
{
    auto& jniCallbackListener = JNICallbackListener::getInstance();

    connect(&jniCallbackListener, &JNICallbackListener::extractTextOk,
            this, [this](QString imgSource, QString text){
                handleExtractTextOk(imgSource, text);
            });

    connect(&jniCallbackListener, &JNICallbackListener::extractTextFailed,
            this, [this](QString imgSource, QString error){
                handleExtractTextFailed(imgSource, error);
            });
}

void ImageUtils::setExtractingText(bool extractingText)
{
    if (extractingText != mExtractingText)
    {
        mExtractingText = extractingText;
        emit extractingTextChanged();
    }
}

bool ImageUtils::extractText(const QString& imgSource)
{
    qDebug() << "Extract text from image:" << imgSource;

    if (mExtractingText)
    {
        qWarning() << "Text extraction still in progress";
        return false;
    }

#if defined(Q_OS_ANDROID)
    QImage img = PhotoPicker::loadImage(imgSource);

    if (img.isNull())
        return false;

    QJniEnvironment env;
    const int size = img.width() * img.height() * 4;
    auto jsImg = env->NewByteArray(size);
    const uint8_t* imgBits = img.constBits();
    env->SetByteArrayRegion(jsImg, 0, size, (jbyte*)imgBits);
    auto jsToken = QJniObject::fromString(imgSource);

    QJniObject::callStaticMethod<void>(
        "com/gmail/mfnboer/TextExtractor",
        "extractText",
        "([BIILjava/lang/String;)V",
        jsImg, (jint)img.width(), (jint)img.height(),
        jsToken.object<jstring>());

    env->DeleteLocalRef(jsImg);
    setExtractingText(true);
    mExtractingTextFromSource = imgSource;

    return true;
#else
    qDebug() << "Text extraction not supported";
    return false;
#endif
}

void ImageUtils::handleExtractTextOk(const QString& source, const QString& text)
{
    if (source != mExtractingTextFromSource)
    {
        qDebug() << "Not for this instance:" << source << "extracting from:" << mExtractingTextFromSource;
        return;
    }

    qDebug() << "Extract text ok:" << source << text;
    setExtractingText(false);
    mExtractingTextFromSource.clear();
    emit extractTextOk(source, text);
}

void ImageUtils::handleExtractTextFailed(const QString& source, const QString& error)
{
    if (source != mExtractingTextFromSource)
    {
        qDebug() << "Not for this instance:" << source << "extracting from:" << mExtractingTextFromSource;
        return;
    }

    qWarning() << "Extract text failed:" << source << error;
    setExtractingText(false);
    mExtractingTextFromSource.clear();
    emit extractTextFailed(source, error);
}

}
