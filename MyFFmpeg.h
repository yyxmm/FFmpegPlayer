#ifndef MYFFMPEG_H
#define MYFFMPEG_H

#include <QObject>

#include <QMutex>
#include <QThread>
#include <QTimer>
#include <QImage>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
}

class MyFFmpeg : public QObject
{
    Q_OBJECT
public:
    explicit MyFFmpeg(QObject *parent = nullptr);
    ~MyFFmpeg();

    void open(const QString &fileName);
    void seek(qint64 pos);
    void setSpeed(qreal speed);

signals:
    void durationChanged(qint64 duration);
    void positionChanged(qint64 position);
    void videoFrame(const QImage &image);

private slots:
    void onThreadStarted();
    void onThreadFinished();

    void onOpen(const QString &fileName);
    void onSeek(qint64 pos);
    void onSetSpeed(qreal speed);

    void onDecodeTimeout();

private:
    QThread m_thread;
    QMutex m_mutex;

    QString m_fileName;
    AVFormatContext *m_formatContext = nullptr;
    int m_videoStreamIndex = -1;
    qint64 m_duration = 0;
    qreal m_fps = 0;
    qreal m_speed = 1;
    AVCodecContext *m_codecContext = nullptr;
    SwsContext *m_swsContext = nullptr;
    AVFrame *m_frame = nullptr;
    AVPacket m_packet;

    QTimer *m_decodeTimer = nullptr;
};

#endif // MYFFMPEG_H
