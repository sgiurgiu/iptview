
#include <QApplication>
#include <QSettings>
#include <QByteArray>
#include <QSslSocket>
#include <QSqlDatabase>
#include <QMessageBox>

#include "iptviewmainwindow.h"

int main(int argc, char** argv)
{
    QCoreApplication::setOrganizationName("Zergiu");
    QCoreApplication::setOrganizationDomain("zergiu.com");
    QCoreApplication::setApplicationName("IPTV View");
    QCoreApplication::setApplicationVersion(IPTVIEW_VERSION);

    QApplication app(argc, argv);
    // Qt sets the locale in the QApplication constructor, but libmpv requires
    // the LC_NUMERIC category to be set to "C", so change it back.
    setlocale(LC_NUMERIC, "C");
    if(!QSslSocket::supportsSsl())
    {
        // we apparently need this song and dance, otherwise just making an
        // https request will fail :(
        // I suppose it's forcing to load openssl before the first
        // connection request is made
        qDebug() << QSslSocket::sslLibraryBuildVersionString();
        qDebug() << QSslSocket::sslLibraryVersionNumber();
        qDebug() << QSslSocket::activeBackend();
    }

    if (!QSqlDatabase::drivers().contains("QSQLITE"))
    {
        QMessageBox::critical(
                    nullptr,
                    "Unable to load database driver",
                    "Cannot find the SQLITE driver"
                    );
        return EXIT_FAILURE;
    }


    IPTViewMainWindow window;
    QSettings settings;
    window.restoreGeometry(settings.value("mainwindow/geometry").value<QByteArray>());
    window.restoreState(settings.value("mainwindow/state").value<QByteArray>());
    window.show();
    return app.exec();
}
