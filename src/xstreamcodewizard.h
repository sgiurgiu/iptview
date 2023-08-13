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
    static CollectedInfo ImportXstreamCodes(QWidget* parent);

};


class XstreamCodeLoginPage : public QWizardPage
{
    Q_OBJECT
    Q_PROPERTY(AuthenticationInfo AuthInfo READ AuthInfo WRITE SetAuthInfo)
public:
    XstreamCodeLoginPage(QWidget* parent = nullptr);
    void initializePage() override;
    bool isComplete() const override;
    AuthenticationInfo AuthInfo() const
    {
        return info;
    }
    void SetAuthInfo(const AuthenticationInfo& info)
    {
        this->info = info;
    }
private slots:
    void verifyInfo();
private:
    QLineEdit* urlEdit;
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QPushButton* verifyButton;
    QNetworkAccessManager* networkManager;
    QLabel* verifiedLabel = nullptr;
    bool isAuthenticated = false;
    AuthenticationInfo info;
};

class XstreamCodeCategoriesPage : public QWizardPage
{
    Q_OBJECT
public:
    XstreamCodeCategoriesPage(QWidget* parent = nullptr);
    bool validatePage() override;
    void initializePage() override;
    const QList<CategoryInfo>& GetLiveCategories() const
    {
        return liveCategories;
    }
    const QList<CategoryInfo>& GetVodCategories() const
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
    void grabLiveCategories(const AuthenticationInfo& info);
    void grabVodCategories(const AuthenticationInfo& info);
private:
    QNetworkAccessManager* networkManager;
    QList<CategoryInfo> liveCategories;
    QList<CategoryInfo> vodCategories;
    QListView* liveCategoriesView;
    QListView* vodCategoriesView;
    QStandardItemModel* liveCategoriesModel;
    QStandardItemModel* vodCategoriesModel;
};


Q_DECLARE_METATYPE(AuthenticationInfo)
Q_DECLARE_METATYPE(CategoryInfo)

#endif // XTREAMCODEWIZARD_H
