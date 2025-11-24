// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#pragma once
#include "enums.h"
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class ReportCategory
{
    Q_GADGET
    Q_PROPERTY(QEnums::ReportCategoryType type READ getType FINAL)
    Q_PROPERTY(QString title READ getTitle FINAL)
    Q_PROPERTY(QString description READ getDescription FINAL)
    QML_VALUE_TYPE(reportcategory)

public:
    using List = QList<ReportCategory>;

    ReportCategory() = default;
    ReportCategory(QEnums::ReportCategoryType categoryType, const QString& title, const QString& description) :
        mCategoryType(categoryType),
        mTitle(title),
        mDescription(description)
    {}

    QEnums::ReportCategoryType getType() const { return mCategoryType; }
    QString getTitle() const { return mTitle; }
    QString getDescription() const { return mDescription; }

private:
    QEnums::ReportCategoryType mCategoryType = QEnums::REPORT_CATEGORY_TYPE_NULL;
    QString mTitle;
    QString mDescription;
};

}

Q_DECLARE_METATYPE(::Skywalker::ReportCategory)
