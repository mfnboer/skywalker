// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "image_utils.h"
#include "jni_callback.h"
#include "photo_picker.h"
#include "shared_image_provider.h"
#include "songlink.h"
#include <QTransform>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

namespace Skywalker {

ImageUtils::ImageUtils(QObject* parent) :
    QObject(parent)
{
    auto& jniCallbackListener = JNICallbackListener::getInstance();

    connect(&jniCallbackListener, &JNICallbackListener::extractTextAvailabilityOk,
            this, [this](QEnums::Script script, bool available){
                handleCheckAvailabilityOk(script, available);
            });

    connect(&jniCallbackListener, &JNICallbackListener::extractTextAvailabilityFailed,
            this, [this](QEnums::Script script, QString error){
                handleCheckAvailabilityFailed(script, error);
            });

    connect(&jniCallbackListener, &JNICallbackListener::extractTextInstallProgress,
            this, [this](QEnums::Script script, int progress){
                handleInstallModuleProgress(script, progress);
            });

    connect(&jniCallbackListener, &JNICallbackListener::extractTextInstallOk,
            this, [this](QEnums::Script script){
                handleInstallModuleOk(script);
            });

    connect(&jniCallbackListener, &JNICallbackListener::extractTextInstallFailed,
            this, [this](QEnums::Script script, QString error){
                handleInstallModuleFailed(script, error);
            });

    connect(&jniCallbackListener, &JNICallbackListener::extractTextOk,
            this, [this](QString imgSource, QString text){
                handleExtractTextOk(imgSource, text);
            });

    connect(&jniCallbackListener, &JNICallbackListener::extractTextFailed,
            this, [this](QString imgSource, QString error){
                handleExtractTextFailed(imgSource, error);
            });
}

void ImageUtils::setInstalling(bool installing)
{
    if (installing != mInstalling)
    {
        mInstalling = installing;
        emit installingChanged();
    }
}

void ImageUtils::setExtractingText(bool extractingText)
{
    if (extractingText != mExtractingText)
    {
        mExtractingText = extractingText;
        emit extractingTextChanged();
    }
}

void ImageUtils::checkAvailability(QEnums::Script script)
{
    qDebug() << "Check availability: " << script;

    if (mInstalling)
    {
        qWarning() << "Installation in progress";
        return;
    }

#if defined(Q_OS_ANDROID)
    QJniObject::callStaticMethod<void>(
        "com/gmail/mfnboer/TextExtractor",
        "checkAvailability",
        "(I)V",
        (jint)script);
    setInstalling(true);
#else
    qDebug() << "Text extraction not supported";
#endif
}

void ImageUtils::installModule(QEnums::Script script)
{
    qDebug() << "Install module: " << script;

    if (mInstalling)
    {
        qWarning() << "Installation in progress";
        return;
    }

#if defined(Q_OS_ANDROID)
    QJniObject::callStaticMethod<void>(
        "com/gmail/mfnboer/TextExtractor",
        "installModule",
        "(I)V",
        (jint)script);
    setInstalling(true);
#else
    qDebug() << "Text extraction not supported";
#endif
}

bool ImageUtils::extractText(QEnums::Script script, const QString& imgSource)
{
    qDebug() << "Extract text from image:" << imgSource << "script:" << script;

    if (mExtractingText)
    {
        qWarning() << "Text extraction still in progress";
        return false;
    }

#if defined(Q_OS_ANDROID)
    QImage img = PhotoPicker::loadImage(imgSource);

    if (img.isNull())
        return false;

    img.convertTo(QImage::Format_RGBA8888); // must match format in TextExtractor.java

    QJniEnvironment env;
    const int size = img.width() * img.height() * 4;
    auto jsImg = env->NewByteArray(size);
    const uint8_t* imgBits = img.constBits();
    env->SetByteArrayRegion(jsImg, 0, (jint)size, (jbyte*)imgBits);
    auto jsToken = QJniObject::fromString(imgSource);

    QJniObject::callStaticMethod<void>(
        "com/gmail/mfnboer/TextExtractor",
        "extractText",
        "(I[BIILjava/lang/String;)V",
        (jint)script, jsImg, (jint)img.width(), (jint)img.height(),
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

void ImageUtils::handleCheckAvailabilityOk(QEnums::Script script, bool available)
{
    setInstalling(false);
    emit checkAvailabilityOk(script, available);
}

void ImageUtils::handleCheckAvailabilityFailed(QEnums::Script script, QString error)
{
    setInstalling(false);
    emit checkAvailabilityFailed(script, error);
}

void ImageUtils::handleInstallModuleProgress(QEnums::Script script, int progressPercentage)
{
    emit installModuleProgress(script, progressPercentage);
}

void ImageUtils::handleInstallModuleOk(QEnums::Script script)
{
    setInstalling(false);
    emit installModuleOk(script);
}

void ImageUtils::handleInstallModuleFailed(QEnums::Script script, QString error)
{
    setInstalling(false);
    emit installModuleFailed(script, error);
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

ImageView ImageUtils::createImageView(const QString& url, const QString& alt)
{
    return ImageView(url, alt);
}

double ImageUtils::getPreferredLinkCardAspectRatio(const QString& link) const
{
    if (Songlink::isMusicLink(link))
        return 1.0;

    const QUrl url(link);

    if (url.host().endsWith("instagram.com"))
        return 1.0;
    if (url.host().endsWith("tiktok.com"))
        return 1000.0 / 690.0;

    return 720.0 / 1280.0;
}

Q_INVOKABLE QString ImageUtils::transformImage(const QString& imgSource, bool horMirror, bool vertMirror, int rotationAngle, const QRect& cutRect) const
{
    qDebug() << "Transform image:" << imgSource << "horMirror:" << horMirror << "vertMirror:" << vertMirror << "rot:" << rotationAngle << "cut:" << cutRect;

    if (!horMirror && !vertMirror && rotationAngle == 0 && cutRect.isEmpty())
    {
        qDebug() << "No transformation";
        return imgSource;
    }

    QImage img = PhotoPicker::loadImage(imgSource);

    if (img.isNull())
    {
        qWarning() << "Cannot load image:" << imgSource;
        return imgSource;
    }

    if (horMirror)
        img = img.mirrored(true, false); // Qt6.9: img.flipped(Qt::Horizontal);

    if (vertMirror)
              img = img.mirrored(false, true); // Qt6.9: img.flipped(Qt::Vertical);

    if (rotationAngle != 0)
        img = img.transformed(QTransform().rotate(rotationAngle));

    if (!cutRect.isEmpty())
        img = img.copy(cutRect);

    auto* imgProvider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);

    if (imgSource.startsWith("image://"))
        imgProvider->removeImage(imgSource);

    const QString newSource = imgProvider->addImage(img);
    qDebug() << "New image source:" << newSource;
    return newSource;
}

}
