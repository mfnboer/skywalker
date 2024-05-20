// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "svg_image.h"
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class SvgFilled : public QObject
{
    Q_OBJECT
    Q_PROPERTY(SvgImage bookmark MEMBER sBookmark CONSTANT FINAL)
    Q_PROPERTY(SvgImage directMessage MEMBER sMail CONSTANT FINAL)
    Q_PROPERTY(SvgImage feed MEMBER sRssFeed CONSTANT FINAL)
    Q_PROPERTY(SvgImage home MEMBER sHome CONSTANT FINAL)
    Q_PROPERTY(SvgImage like MEMBER sFavorite CONSTANT FINAL)
    Q_PROPERTY(SvgImage list MEMBER sList CONSTANT FINAL)
    Q_PROPERTY(SvgImage lock MEMBER sLock CONSTANT FINAL)
    Q_PROPERTY(SvgImage moderator MEMBER sShield CONSTANT FINAL)
    Q_PROPERTY(SvgImage newFollower MEMBER sPersonAdd CONSTANT FINAL)
    Q_PROPERTY(SvgImage notifications MEMBER sNotifications CONSTANT FINAL)
    Q_PROPERTY(SvgImage search MEMBER sSearch CONSTANT FINAL)
    Q_PROPERTY(SvgImage star MEMBER sStar CONSTANT FINAL)
    Q_PROPERTY(SvgImage unknownAvatar MEMBER sPerson CONSTANT FINAL)
    QML_ELEMENT

public:
    explicit SvgFilled(QObject* parent = nullptr) : QObject(parent) {}

private:
    // fonts.google.com weight=100, grade=0, optical size=24px
    static constexpr SvgImage sBookmark{"M252-198v-530q0-26 17-43t43-17h336q26 0 43 17t17 43v530l-228-98-228 98Z"};
    static constexpr SvgImage sFavorite{"m480-190-22-20q-97-89-160.5-152t-100-110.5Q161-520 146.5-558T132-634q0-71 48.5-119.5T300-802q53 0 99 28.5t81 83.5q35-55 81-83.5t99-28.5q71 0 119.5 48.5T828-634q0 38-14.5 76t-51 85.5Q726-425 663-362T502-210l-22 20Z"};
    static constexpr SvgImage sHome{"M212-172v-402l268-203 268 203v402H550v-248H410v248H212Z"};
    static constexpr SvgImage sList{"M280-600v-80h560v80H280Zm0 160v-80h560v80H280Zm0 160v-80h560v80H280ZM160-600q-17 0-28.5-11.5T120-640q0-17 11.5-28.5T160-680q17 0 28.5 11.5T200-640q0 17-11.5 28.5T160-600Zm0 160q-17 0-28.5-11.5T120-480q0-17 11.5-28.5T160-520q17 0 28.5 11.5T200-480q0 17-11.5 28.5T160-440Zm0 160q-17 0-28.5-11.5T120-320q0-17 11.5-28.5T160-360q17 0 28.5 11.5T200-320q0 17-11.5 28.5T160-280Z"}; // NOT FILLED (weight=400)
    static constexpr SvgImage sLock{"M272-132q-24.75 0-42.375-17.625T212-192v-336q0-24.75 17.625-42.375T272-588h60v-80q0-62 43-105t105-43q62 0 105 43t43 105v80h60q24.75 0 42.375 17.625T748-528v336q0 24.75-17.625 42.375T688-132H272Zm208-174q23 0 38.5-15.5T534-360q0-23-15.5-38.5T480-414q-23 0-38.5 15.5T426-360q0 23 15.5 38.5T480-306ZM360-588h240v-80q0-50-35-85t-85-35q-50 0-85 35t-35 85v80Z"};
    static constexpr SvgImage sMail{"M192-212q-26 0-43-17t-17-43v-416q0-26 17-43t43-17h576q26 0 43 17t17 43v416q0 26-17 43t-43 17H192Zm288-274 320-212-16-22-304 200-304-200-16 22 320 212Z"};
    static constexpr SvgImage sNotifications{"M212-212v-28h60v-328q0-77 49.5-135T446-774v-20q0-14.167 9.882-24.083 9.883-9.917 24-9.917Q494-828 504-818.083q10 9.916 10 24.083v20q75 13 124.5 71T688-568v328h60v28H212Zm267.823 100Q455-112 437.5-129.625T420-172h120q0 25-17.677 42.5t-42.5 17.5Z"};
    static constexpr SvgImage sPerson{"M480-512q-44.55 0-76.275-31.725Q372-575.45 372-620q0-44.55 31.725-76.275Q435.45-728 480-728q44.55 0 76.275 31.725Q588-664.55 588-620q0 44.55-31.725 76.275Q524.55-512 480-512ZM212-232v-52q0-22 13.5-41.5T262-356q55-26 109.5-39T480-408q54 0 108.5 13T698-356q23 11 36.5 30.5T748-284v52H212Z"};
    static constexpr SvgImage sPersonAdd{"M733-426v-120H613v-28h120v-120h28v120h120v28H761v120h-28Zm-373-86q-44.55 0-76.275-31.725Q252-575.45 252-620q0-44.55 31.725-76.275Q315.45-728 360-728q44.55 0 76.275 31.725Q468-664.55 468-620q0 44.55-31.725 76.275Q404.55-512 360-512ZM92-232v-52q0-22 13.5-41.5T142-356q55-26 109.5-39T360-408q54 0 108.5 13T578-356q23 11 36.5 30.5T628-284v52H92Z"};
    static constexpr SvgImage sRssFeed{"M200-120q-33 0-56.5-23.5T120-200q0-33 23.5-56.5T200-280q33 0 56.5 23.5T280-200q0 33-23.5 56.5T200-120Zm480 0q0-117-44-218.5T516-516q-76-76-177.5-120T120-680v-120q142 0 265 53t216 146q93 93 146 216t53 265H680Zm-240 0q0-67-25-124.5T346-346q-44-44-101.5-69T120-440v-120q92 0 171.5 34.5T431-431q60 60 94.5 139.5T560-120H440Z"}; // NOT FILLED (weight=400)
    static constexpr SvgImage sSearch{"M784-120 532-372q-30 24-69 38t-83 14q-109 0-184.5-75.5T120-580q0-109 75.5-184.5T380-840q109 0 184.5 75.5T640-580q0 44-14 83t-38 69l252 252-56 56ZM380-400q75 0 127.5-52.5T560-580q0-75-52.5-127.5T380-760q-75 0-127.5 52.5T200-580q0 75 52.5 127.5T380-400Z"}; // NOT FILLED (weight=400)
    static constexpr SvgImage sShield{"M480-134q-115-36-191.5-142T212-516v-208l268-100 268 100v208q0 134-76.5 240T480-134Z"};
    static constexpr SvgImage sStar{"m311-188 45-192-149-129 196-17 77-181 77 181 196 17-149 129 45 192-169-102-169 102Z"};
};

}
