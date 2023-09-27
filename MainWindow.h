#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "MyFFmpeg.h"
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onDurationChanged(qint64 duration);
    void onPositionChanged(qint64 position);
    void onVideoFrame(const QImage &image);

    void on_pushButtonOpen_clicked();
    void on_horizontalSlider_sliderPressed();
    void on_horizontalSlider_sliderReleased();
    void on_horizontalSlider_valueChanged(int value);
    void on_pushButtonPlay_clicked();
    void on_pushButtonPause_clicked();
    void on_pushButtonSpeedDown_clicked();
    void on_pushButtonSpeedUp_clicked();

private:
    Ui::MainWindow *ui;

    MyFFmpeg *m_ffmpeg;
    QSettings *m_settings;

    bool m_sliderPressed = false;
    qreal m_speed = 1;
};
#endif // MAINWINDOW_H
