#ifndef LABELVIDEO_H
#define LABELVIDEO_H

#include <QLabel>

class LabelVideo : public QLabel
{
    Q_OBJECT
public:
    explicit LabelVideo(QWidget *parent = nullptr);

    void showImage(const QImage &image);

protected:
    void paintEvent(QPaintEvent *event) override;

signals:

private:
    QImage m_image;
};

#endif // LABELVIDEO_H
