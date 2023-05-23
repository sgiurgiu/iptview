#ifndef IPTVIEWMAINWINDOW_H
#define IPTVIEWMAINWINDOW_H

#include <QMainWindow>

class IPTViewMainWidget;

class IPTViewMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit IPTViewMainWindow(QWidget *parent = nullptr);

signals:
protected:
  void closeEvent(QCloseEvent * event) override;
private slots:
  void openPlaylist();
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

};

#endif // IPTVIEWMAINWINDOW_H