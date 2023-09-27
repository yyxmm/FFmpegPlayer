#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_settings = new QSettings(QString("%1/settings.ini").arg(QCoreApplication::applicationDirPath()), QSettings::IniFormat, this);
    ui->lineEditPath->setText(m_settings->value("path").toString());

    m_ffmpeg = new MyFFmpeg();
    connect(m_ffmpeg, &MyFFmpeg::durationChanged, this, &MainWindow::onDurationChanged);
    connect(m_ffmpeg, &MyFFmpeg::positionChanged, this, &MainWindow::onPositionChanged);
    connect(m_ffmpeg, &MyFFmpeg::videoFrame, this, &MainWindow::onVideoFrame);
}

MainWindow::~MainWindow()
{
    delete m_ffmpeg;

    delete ui;
}

void MainWindow::onDurationChanged(qint64 duration)
{
    ui->horizontalSlider->setRange(0, duration);

    ui->labelPosition->setText("00:00:00");
    ui->labelDuration->setText(QString("%1:%2:%3").arg(duration / 60 / 60, 2, 10, QLatin1Char('0'))
                                                      .arg(duration / 60 % 60, 2, 10, QLatin1Char('0'))
                                                      .arg(duration % 60, 2, 10, QLatin1Char('0')));
}

void MainWindow::onPositionChanged(qint64 position)
{
    if (m_sliderPressed) {
        return;
    }

    ui->horizontalSlider->blockSignals(true);
    ui->horizontalSlider->setValue(position);
    ui->horizontalSlider->blockSignals(false);

    ui->labelPosition->setText(QString("%1:%2:%3").arg(position / 60 / 60, 2, 10, QLatin1Char('0'))
                                                  .arg(position / 60 % 60, 2, 10, QLatin1Char('0'))
                                                  .arg(position % 60, 2, 10, QLatin1Char('0')));
}

void MainWindow::onVideoFrame(const QImage &image)
{
    ui->labelVideo->showImage(image);
}

void MainWindow::on_pushButtonOpen_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Open File", ui->lineEditPath->text());
    if (path.isEmpty()) {
        return;
    }

    ui->lineEditPath->setText(path);
    m_settings->setValue("path", path);
}

void MainWindow::on_horizontalSlider_sliderPressed()
{
    m_sliderPressed = true;
}

void MainWindow::on_horizontalSlider_sliderReleased()
{
    m_sliderPressed = false;
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    m_ffmpeg->seek(value);
}

void MainWindow::on_pushButtonPlay_clicked()
{
    QString path = ui->lineEditPath->text();
    m_ffmpeg->open(path);
}

void MainWindow::on_pushButtonPause_clicked()
{

}

void MainWindow::on_pushButtonSpeedUp_clicked()
{
    m_speed *= 2;
    m_ffmpeg->setSpeed(m_speed);
    ui->labelSpeed->setText(QString("x%1").arg(m_speed));
}

void MainWindow::on_pushButtonSpeedDown_clicked()
{
    m_speed /= 2;
    m_ffmpeg->setSpeed(m_speed);
    ui->labelSpeed->setText(QString("x%1").arg(m_speed));
}
