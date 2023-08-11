#ifndef XSTREAMINFO_H
#define XSTREAMINFO_H

#include <QString>
#include <QVariant>
#include <QList>

struct AuthenticationInfo
{
    QString username;
    QString password;
    QVariantList outputFormats;
    QString serverUrl;
    QString serverPort;
    QString httpsPort;
    QString serverSchema;
    QString rtmpPort;
};

struct CategoryInfo
{
    QString categoryId;
    QString categoryName;
};

struct CollectedInfo
{
    AuthenticationInfo authInfo;
    QList<CategoryInfo> liveCategories;
    QList<CategoryInfo> vodCategories;
};

#endif // XSTREAMINFO_H
