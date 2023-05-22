#ifndef CHANNELSWIDGET_H
#define CHANNELSWIDGET_H

#include <QWidget>

class QTreeView;

class ChannelsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChannelsWidget(QWidget *parent = nullptr);

signals:
private:
    QTreeView* channels;
};

#endif // CHANNELSWIDGET_H
