#include "mediawidget.h"
#include <QVBoxLayout>
#include <QPushButton>

#include "mpvwidget.h"

MediaWidget::MediaWidget(QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    mpvWidget = new MpvWidget(this);
    playButton = new QPushButton(this);
    playButton->setText("Play");
    layout->addWidget(mpvWidget, 1);
    layout->addWidget(playButton, 0);

    setLayout(layout);
    connect(playButton, &QPushButton::clicked, this, &MediaWidget::play);
}

void MediaWidget::play()
{

}

void MediaWidget::PlayChannel(QString uri)
{
    mpvWidget->command(QStringList() << "loadfile" << uri);
}
