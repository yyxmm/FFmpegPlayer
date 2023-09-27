#include "LabelVideo.h"

#include <QPainter>

LabelVideo::LabelVideo(QWidget *parent)
    : QLabel{parent}
{

}

void LabelVideo::showImage(const QImage &image)
{
    m_image = image;
    update();
}

void LabelVideo::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);

    QPainter painter{this};
    painter.drawImage(rect(), m_image);
}
