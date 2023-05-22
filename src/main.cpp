
#include <QApplication>
#include <QSettings>
#include <QByteArray>

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
    IPTViewMainWindow window;
    QSettings settings;
    window.restoreGeometry(settings.value("mainwindow/geometry").value<QByteArray>());
    window.restoreState(settings.value("mainwindow/state").value<QByteArray>());
    window.show();
    return app.exec();
}
