#include "xstreamcodewizard.h"

#include <QWizard>
#include <QWizardPage>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QPromise>
#include <QFuture>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QListView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

XstreamCodeLoginPage::XstreamCodeLoginPage(QNetworkAccessManager* networkManager,QWidget* parent):QWizardPage{parent},
    networkManager{networkManager}
{
    setTitle("Xstream-Code Login Information");
    QLabel* urlLabel = new QLabel("URL:");
    urlEdit = new QLineEdit;
    urlEdit->setPlaceholderText("http://example.com");
    QLabel* usernameLabel = new QLabel("Username:");
    usernameEdit = new QLineEdit;
    QLabel* passwordLabel = new QLabel("Password:");
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    verifiedLabel = new QLabel("Not verified");
    verifyButton = new QPushButton("Verify", this);
    connect(verifyButton, SIGNAL(clicked()), this, SLOT(verifyInfo()));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(urlLabel, 0, 0);
    layout->addWidget(urlEdit, 0, 1);
    layout->addWidget(usernameLabel, 1, 0);
    layout->addWidget(usernameEdit, 1, 1);
    layout->addWidget(passwordLabel, 2, 0);
    layout->addWidget(passwordEdit, 2, 1);
    layout->addWidget(verifyButton, 3, 0);
    layout->addWidget(verifiedLabel, 3, 1);
    setLayout(layout);

    registerField("info", this, "AuthInfo");
}
void XstreamCodeLoginPage::verifyInfo()
{
    verifyButton->setEnabled(false);
    QNetworkRequest request;
    QUrl url{urlEdit->text()};
    url.setPath("/player_api.php");
    QUrlQuery query;
    query.addQueryItem("username",usernameEdit->text());
    query.addQueryItem("password",passwordEdit->text());
    url.setQuery(query);
    request.setUrl(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "IPTView 1.0");
    auto reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [reply, this]()
    {
        if(reply->error())
        {
            reply->deleteLater();
            return;
        }
        auto response = reply->readAll();
        reply->deleteLater();

        QJsonDocument doc = QJsonDocument::fromJson(response);
        auto object = doc.object();
        AuthenticationInfo info;
        if(object.contains("user_info"))
        {
            auto userInfo = object["user_info"].toObject();
            auto isAuthenticatedValue = userInfo.value("auth");
            bool isAuthenticated = isAuthenticatedValue.toInteger(0) == 1;
            bool isActive = userInfo.value("status").toString() == "Active";
            this->isAuthenticated = isAuthenticated && isActive;
            verifiedLabel->setText(this->isAuthenticated ? "Verified" : "Verification failed");
            verifyButton->setEnabled(true);
            info.username = userInfo.value("username").toString("");
            info.password = userInfo.value("password").toString("");
            info.outputFormats = userInfo.value("allowed_output_formats").toArray({}).toVariantList();
        }
        if(object.contains("server_info"))
        {
            auto serverInfo = object["server_info"].toObject();
            info.serverUrl = serverInfo.value("url").toString("");
            info.serverPort = serverInfo.value("port").toString("");
            info.httpsPort = serverInfo.value("https_port").toString("");
            info.rtmpPort = serverInfo.value("rtmp_port").toString("");
            info.serverSchema = serverInfo.value("server_protocol").toString("");
        }
        QVariant infoVariant;
        this->SetAuthInfo(info);
        infoVariant.setValue(info);
        this->setField("info",infoVariant);
        emit completeChanged();
    });
}

bool XstreamCodeLoginPage::isComplete() const
{
    return isAuthenticated;
}

void XstreamCodeLoginPage::initializePage()
{
    usernameEdit->setFocus();
    auto infoField = field("info").value<AuthenticationInfo>();
    usernameEdit->setText(infoField.username);
    passwordEdit->setText(infoField.password);
    if(!info.serverUrl.isEmpty())
    {
        QUrl url;
        url.setHost(info.serverUrl);
        url.setScheme(info.serverSchema);
        url.setPort(info.serverPort.toInt());
        urlEdit->setText(url.toString());
    }
}

XstreamCodeCategoriesPage::XstreamCodeCategoriesPage(QNetworkAccessManager* networkManager,QWidget* parent):QWizardPage{parent},
    networkManager{networkManager}
{
    setTitle("Select categories");
    QWidget* liveCategoriesTab = new QWidget();
    QVBoxLayout* liveCategoriesLayout = new QVBoxLayout(this);
    liveCategoriesView = new QListView();
    liveCategoriesModel = new QStandardItemModel(liveCategoriesView);
    liveCategoriesView->setModel(liveCategoriesModel);
    {
        QPushButton* selectAllButton = new QPushButton("Select all");
        QPushButton* deselectAllButton = new QPushButton("Deselect all");
        QPushButton* invertSelectionButton = new QPushButton("Invert selection");
        QHBoxLayout* buttonsLayout = new QHBoxLayout();
        buttonsLayout->addWidget(selectAllButton);
        buttonsLayout->addWidget(deselectAllButton);
        buttonsLayout->addWidget(invertSelectionButton);
        connect(selectAllButton, SIGNAL(clicked()), this, SLOT(selectAllLive()));
        connect(deselectAllButton, SIGNAL(clicked()), this, SLOT(deselectAllLive()));
        connect(invertSelectionButton, SIGNAL(clicked()), this, SLOT(invertSelectionLive()));
        liveCategoriesLayout->addLayout(buttonsLayout);
    }
    liveCategoriesLayout->addWidget(liveCategoriesView, 1);
    liveCategoriesTab->setLayout(liveCategoriesLayout);

    QWidget* vodCategoriesTab = new QWidget();
    QVBoxLayout* vodCategoriesLayout = new QVBoxLayout(this);
    vodCategoriesView = new QListView;
    vodCategoriesModel = new QStandardItemModel(vodCategoriesView);
    vodCategoriesView->setModel(vodCategoriesModel);
    {
        QPushButton* selectAllButton = new QPushButton("Select all");
        QPushButton* deselectAllButton = new QPushButton("Deselect all");
        QPushButton* invertSelectionButton = new QPushButton("Invert selection");
        QHBoxLayout* buttonsLayout = new QHBoxLayout();
        buttonsLayout->addWidget(selectAllButton);
        buttonsLayout->addWidget(deselectAllButton);
        buttonsLayout->addWidget(invertSelectionButton);
        connect(selectAllButton, SIGNAL(clicked()), this, SLOT(selectAllVod()));
        connect(deselectAllButton, SIGNAL(clicked()), this, SLOT(deselectAllVod()));
        connect(invertSelectionButton, SIGNAL(clicked()), this, SLOT(invertSelectionVod()));
        vodCategoriesLayout->addLayout(buttonsLayout);
    }
    vodCategoriesLayout->addWidget(vodCategoriesView, 1);
    vodCategoriesTab->setLayout(vodCategoriesLayout);

    QTabWidget* tab = new QTabWidget(this);
    tab->addTab(liveCategoriesTab, "Live Streams");
    tab->addTab(vodCategoriesTab, "VOD Streams");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(tab,1);
    setLayout(layout);
    setFinalPage(true);
}
void XstreamCodeCategoriesPage::initializePage()
{
    auto infoField = field("info").value<AuthenticationInfo>();
    grabLiveCategories(infoField);
    grabVodCategories(infoField);
}

void XstreamCodeCategoriesPage::grabLiveCategories(const AuthenticationInfo& info)
{
    QNetworkRequest request;
    QUrl url;
    url.setHost(info.serverUrl);
    url.setScheme(info.serverSchema);
    url.setPort(info.serverPort.toInt());
    url.setPath("/player_api.php");
    QUrlQuery query;
    query.addQueryItem("username",info.username);
    query.addQueryItem("password",info.password);
    query.addQueryItem("action","get_live_categories");
    url.setQuery(query);
    request.setUrl(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "IPTView 1.0");
    auto reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [reply, this]()
    {
        if(reply->error())
        {
            reply->deleteLater();
            return;
        }
        auto response = reply->readAll();
        reply->deleteLater();

        QJsonDocument doc = QJsonDocument::fromJson(response);
        auto categories = doc.array();
        for(const auto& cat : categories)
        {
            if(!cat.isObject()) continue;
            auto catObject = cat.toObject();
            CategoryInfo catInfo;
            catInfo.categoryId = catObject.value("category_id").toString("");
            catInfo.categoryName = catObject.value("category_name").toString("");
            QStandardItem* standardItem = new QStandardItem(catInfo.categoryName);
            standardItem->setCheckable(true);
            standardItem->setCheckState(Qt::CheckState::Unchecked);
            standardItem->setEditable(false);
            standardItem->setData(QVariant::fromValue(catInfo));
            liveCategoriesModel->appendRow(standardItem);
        }
    });
}
void XstreamCodeCategoriesPage::grabVodCategories(const AuthenticationInfo& info)
{
    QNetworkRequest request;
    QUrl url;
    url.setHost(info.serverUrl);
    url.setScheme(info.serverSchema);
    url.setPort(info.serverPort.toInt());
    url.setPath("/player_api.php");
    QUrlQuery query;
    query.addQueryItem("username",info.username);
    query.addQueryItem("password",info.password);
    query.addQueryItem("action","get_vod_categories");
    url.setQuery(query);
    request.setUrl(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "IPTView 1.0");
    auto reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [reply, this]()
    {
        if(reply->error())
        {
            reply->deleteLater();
            return;
        }
        auto response = reply->readAll();
        reply->deleteLater();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        auto categories = doc.array();
        for(const auto& cat : categories)
        {
            if(!cat.isObject()) continue;
            auto catObject = cat.toObject();
            CategoryInfo catInfo;
            catInfo.categoryId = catObject.value("category_id").toString("");
            catInfo.categoryName = catObject.value("category_name").toString("");
            QStandardItem* standardItem = new QStandardItem(catInfo.categoryName);
            standardItem->setData(QVariant::fromValue(catInfo));
            standardItem->setCheckable(true);
            standardItem->setCheckState(Qt::CheckState::Unchecked);
            standardItem->setEditable(false);
            vodCategoriesModel->appendRow(standardItem);
        }
    });
}

void XstreamCodeCategoriesPage::selectAllLive()
{
    for(int i=0;i<liveCategoriesModel->rowCount();i++)
    {
        auto item = liveCategoriesModel->item(i);
        item->setCheckState(Qt::CheckState::Checked);
    }
}
void XstreamCodeCategoriesPage::deselectAllLive()
{
    for(int i=0;i<liveCategoriesModel->rowCount();i++)
    {
        auto item = liveCategoriesModel->item(i);
        item->setCheckState(Qt::CheckState::Unchecked);
    }
}
void XstreamCodeCategoriesPage::invertSelectionLive()
{
    for(int i=0;i<liveCategoriesModel->rowCount();i++)
    {
        auto item = liveCategoriesModel->item(i);
        auto checkState = item->checkState();
        item->setCheckState(checkState == Qt::CheckState::Checked ? Qt::CheckState::Unchecked : Qt::CheckState::Checked);
    }
}
void XstreamCodeCategoriesPage::selectAllVod()
{
    for(int i=0;i<vodCategoriesModel->rowCount();i++)
    {
        auto item = vodCategoriesModel->item(i);
        item->setCheckState(Qt::CheckState::Checked);
    }
}
void XstreamCodeCategoriesPage::deselectAllVod()
{
    for(int i=0;i<vodCategoriesModel->rowCount();i++)
    {
        auto item = vodCategoriesModel->item(i);
        item->setCheckState(Qt::CheckState::Unchecked);
    }
}
void XstreamCodeCategoriesPage::invertSelectionVod()
{
    for(int i=0;i<vodCategoriesModel->rowCount();i++)
    {
        auto item = vodCategoriesModel->item(i);
        auto checkState = item->checkState();
        item->setCheckState(checkState == Qt::CheckState::Checked ? Qt::CheckState::Unchecked : Qt::CheckState::Checked);
    }
}

bool XstreamCodeCategoriesPage::validatePage()
{

    for(int i=0;i<liveCategoriesModel->rowCount();i++)
    {
        auto item = liveCategoriesModel->item(i);
        auto checkState = item->checkState();
        if(checkState == Qt::CheckState::Checked)
        {
            liveCategories.append(item->data().value<CategoryInfo>());
        }
    }
    for(int i=0;i<vodCategoriesModel->rowCount();i++)
    {
        auto item = vodCategoriesModel->item(i);
        auto checkState = item->checkState();
        if(checkState == Qt::CheckState::Checked)
        {
            vodCategories.append(item->data().value<CategoryInfo>());
        }
    }

    return !(liveCategories.empty() && vodCategories.empty());
}

CollectedInfo XstreamCodeWizard::ImportXstreamCodes(QWidget* parent,QNetworkAccessManager* networkManager)
{
    QWizard wizard(parent, Qt::Dialog);
    wizard.setModal(true);
    wizard.setWindowTitle("Import Xtream-Code Playlist");
    wizard.addPage(new XstreamCodeLoginPage(networkManager));
    auto categoriesPage = new XstreamCodeCategoriesPage(networkManager);
    wizard.addPage(categoriesPage);
    CollectedInfo info;
    if(wizard.exec() == QDialog::Accepted)
    {
        info.authInfo = wizard.field("info").value<AuthenticationInfo>();
        info.liveCategories = categoriesPage->GetLiveCategories();
        info.vodCategories = categoriesPage->GetVodCategories();
    }
    return info;
}
