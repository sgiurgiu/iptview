#ifndef XSTREAMINFO_H
#define XSTREAMINFO_H

#include <QString>
#include <QVariant>
#include <QList>

struct XStreamAuthenticationInfo
{
    QString username;
    QString password;
    QVariantList outputFormats;
    QString serverUrl;
    QString serverPort;
    QString httpsPort;
    QString serverSchema;
    QString rtmpPort;
    QString timezone;
    int64_t id;
};

struct XStreamCategoryInfo
{
    QString categoryId;
    QString categoryName;
};

struct XStreamCollectedInfo
{
    XStreamAuthenticationInfo authInfo;
    QList<XStreamCategoryInfo> liveCategories;
    QList<XStreamCategoryInfo> vodCategories;
};

#endif // XSTREAMINFO_H
