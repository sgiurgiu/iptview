#ifndef XTREAMCODEWIZARD_H
#define XTREAMCODEWIZARD_H

#include <QWizard>
#include <QMetaType>

#include "xstreaminfo.h"

class QLineEdit;
class QNetworkAccessManager;
class QLabel;
class QListView;
class QStandardItemModel;

class XstreamCodeWizard
{
public:
    static XStreamCollectedInfo ImportXstreamCodes(QWidget *parent, QNetworkAccessManager *networkManager);
};

class XstreamCodeLoginPage : public QWizardPage
{
    Q_OBJECT
    Q_PROPERTY(XStreamAuthenticationInfo AuthInfo READ AuthInfo WRITE SetAuthInfo)
public:
    XstreamCodeLoginPage(QNetworkAccessManager *networkManager, QWidget *parent = nullptr);
    void initializePage() override;
    bool isComplete() const override;
    XStreamAuthenticationInfo AuthInfo() const
    {
        return info;
    }
    void SetAuthInfo(const XStreamAuthenticationInfo &info)
    {
        this->info = info;
    }
private slots:
    void verifyInfo();

private:
    QLineEdit *urlEdit;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *verifyButton;
    QNetworkAccessManager *networkManager;
    QLabel *verifiedLabel = nullptr;
    bool isAuthenticated = false;
    XStreamAuthenticationInfo info;

};

class XstreamCodeCategoriesPage : public QWizardPage
{
    Q_OBJECT
public:
    XstreamCodeCategoriesPage(QNetworkAccessManager *networkManager, QWidget *parent = nullptr);
    bool validatePage() override;
    void initializePage() override;
    const QList<XStreamCategoryInfo> &GetLiveCategories() const
    {
        return liveCategories;
    }
    const QList<XStreamCategoryInfo> &GetVodCategories() const
    {
        return vodCategories;
    }
private slots:
    void selectAllLive();
    void deselectAllLive();
    void invertSelectionLive();
    void selectAllVod();
    void deselectAllVod();
    void invertSelectionVod();

private:
    void grabLiveCategories(const XStreamAuthenticationInfo &info);
    void grabVodCategories(const XStreamAuthenticationInfo &info);

private:
    QNetworkAccessManager *networkManager;
    QList<XStreamCategoryInfo> liveCategories;
    QList<XStreamCategoryInfo> vodCategories;
    QListView *liveCategoriesView;
    QListView *vodCategoriesView;
    QStandardItemModel *liveCategoriesModel;
    QStandardItemModel *vodCategoriesModel;
};

Q_DECLARE_METATYPE(XStreamAuthenticationInfo)
Q_DECLARE_METATYPE(XStreamCategoryInfo)

#endif // XTREAMCODEWIZARD_H
