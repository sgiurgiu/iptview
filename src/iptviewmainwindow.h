#ifndef IPTVIEWMAINWINDOW_H
#define IPTVIEWMAINWINDOW_H

#include <QMainWindow>
#include "m3ulist.h"

class IPTViewMainWidget;

class IPTViewMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit IPTViewMainWindow(QWidget *parent = nullptr);

signals:
    void importPlaylist(M3UList);
protected:
  void closeEvent(QCloseEvent * event) override;
private slots:
  void openPlaylist();
  void fullScreen(bool flag);
  void onImportPlaylist(M3UList list);
private:
  void centerOnScreen();
  void createActions();
  void createMenus();
  void addStatusBar();
  void addWindowWidgets();
  void loadPlaylist(const QString& fileName);
private:
  QMenu* viewMenu;
  QAction* fileNewAction;
  QAction* fileOpenAction;
  QAction* quitApplicationAction;
  IPTViewMainWidget* mainWidget;
  QMargins contentMargins;
};

#endif // IPTVIEWMAINWINDOW_H
