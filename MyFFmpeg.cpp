#include "MyFFmpeg.h"

#include <QElapsedTimer>
#include <QtDebug>

MyFFmpeg::MyFFmpeg(QObject *parent) :
    QObject{parent}
{
    moveToThread(&m_thread);
    connect(&m_thread, &QThread::started, this, &MyFFmpeg::onThreadStarted);
    connect(&m_thread, &QThread::finished, this, &MyFFmpeg::onThreadFinished);
    m_thread.start();
}

MyFFmpeg::~MyFFmpeg()
{
    m_thread.quit();
    m_thread.wait();
}

void MyFFmpeg::open(const QString &fileName)
{
    QMetaObject::invokeMethod(this, "onOpen", Qt::QueuedConnection, Q_ARG(QString, fileName));
}

void MyFFmpeg::seek(qint64 pos)
{
    QMetaObject::invokeMethod(this, "onSeek", Qt::QueuedConnection, Q_ARG(qint64, pos));
}

void MyFFmpeg::setSpeed(qreal speed)
{
    QMetaObject::invokeMethod(this, "onSetSpeed", Qt::QueuedConnection, Q_ARG(qreal, speed));
}

void MyFFmpeg::onThreadStarted()
{
    m_decodeTimer = new QTimer(this);
    m_decodeTimer->setInterval(10);
    connect(m_decodeTimer, &QTimer::timeout, this, &MyFFmpeg::onDecodeTimeout);
}

void MyFFmpeg::onThreadFinished()
{
}

void MyFFmpeg::onOpen(const QString &fileName)
{
    m_fileName = fileName;

    // 先停止
    m_decodeTimer->stop();
    if (m_formatContext) {
        avformat_close_input(&m_formatContext);
    }

    int ret = avformat_open_input(&m_formatContext, fileName.toUtf8().constData(), nullptr, nullptr);
    if (ret < 0) {
        // 打开文件失败
        qDebug() << "avformat_open_input failed";
        return;
    }

    ret = avformat_find_stream_info(m_formatContext, nullptr);
    if (ret < 0) {
        // 获取流信息失败
        qDebug() << "avformat_find_stream_info failed";
        return;
    }

    m_videoStreamIndex = -1;
    AVCodecParameters *codecParameters = nullptr;
    for (uint i = 0; i < m_formatContext->nb_streams; ++i) {
        AVStream *stream = m_formatContext->streams[i];
        AVCodecParameters *codecpar = stream->codecpar;
        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_videoStreamIndex = i;
            codecParameters = codecpar;
            break;
        }
    }
    if (m_videoStreamIndex == -1 || !codecParameters) {
        // 找不到视频流
        qDebug() << "videoStreamIndex == -1 || !codecParameters";
        avformat_close_input(&m_formatContext);
        return;
    }

    AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
    if (!codec) {
        // 找不到解码器
        qDebug() << "avcodec_find_decoder failed";
        avformat_close_input(&m_formatContext);
        return;
    }

    m_codecContext = avcodec_alloc_context3(codec);
    if (!m_codecContext) {
        // 分配解码器上下文失败
        qDebug() << "avcodec_alloc_context3 failed";
        avformat_close_input(&m_formatContext);
        return;
    }

    ret = avcodec_parameters_to_context(m_codecContext, codecParameters);
    if (ret < 0) {
        // 从参数设置解码器上下文失败
        qDebug() << "avcodec_parameters_to_context failed";
        avcodec_free_context(&m_codecContext);
        avformat_close_input(&m_formatContext);
        return;
    }

    ret = avcodec_open2(m_codecContext, codec, nullptr);
    if (ret < 0) {
        // 打开解码器失败
        qDebug() << "avcodec_open2 failed";
        avcodec_free_context(&m_codecContext);
        avformat_close_input(&m_formatContext);
        return;
    }

    // 视频时长
    m_duration = m_formatContext->duration / AV_TIME_BASE;
    qDebug() << "duration:" << m_duration << "s";
    emit durationChanged(m_duration);
    // 视频帧率
    m_fps = av_q2d(m_formatContext->streams[m_videoStreamIndex]->avg_frame_rate);
    qDebug() << "fps:" << m_fps;

    if (!m_swsContext) {
        m_swsContext = sws_getContext(m_codecContext->width, m_codecContext->height, m_codecContext->pix_fmt,
                                      m_codecContext->width, m_codecContext->height, AV_PIX_FMT_RGB32,
                                      SWS_BICUBIC, nullptr, nullptr, nullptr);
        if (!m_swsContext) {
            qDebug() << "sws_getContext failed";
            avcodec_free_context(&m_codecContext);
            avformat_close_input(&m_formatContext);
            return;
        }
    }

    if (!m_frame) {
        m_frame = av_frame_alloc();
    }

    m_decodeTimer->start(0);
}

void MyFFmpeg::onSeek(qint64 pos)
{
    if (pos < 0 || pos > m_duration) {
        return;
    }

    qint64 pts = pos * (m_formatContext->streams[m_videoStreamIndex]->time_base.den) / (m_formatContext->streams[m_videoStreamIndex]->time_base.num);
    int ret = av_seek_frame(m_formatContext, m_videoStreamIndex, pts, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        qDebug() << "av_seek_frame failed";
        return;
    }
}

void MyFFmpeg::onSetSpeed(qreal speed)
{
    m_speed = speed;
}

void MyFFmpeg::onDecodeTimeout()
{
    QElapsedTimer timer;
    timer.start();

    while (av_read_frame(m_formatContext, &m_packet) >= 0) {
        if (m_packet.stream_index == m_videoStreamIndex) {
            int ret = avcodec_send_packet(m_codecContext, &m_packet);
            if (ret < 0) {
                // 发送包给解码器失败
                qCritical() << "avcodec_send_packet failed";
                break;
            }
            //qDebug() << m_packet.pts << m_packet.dts << m_packet.duration;
            qint64 seconds = m_packet.pts * (m_formatContext->streams[m_videoStreamIndex]->time_base.num) / (m_formatContext->streams[m_videoStreamIndex]->time_base.den);
            emit positionChanged(seconds);

            while (ret >= 0) {
                ret = avcodec_receive_frame(m_codecContext, m_frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    // 从解码器接收帧失败
                    //qDebug() << "avcodec_receive_frame failed";
                    break;
                } else if (ret < 0) {
                    // 从解码器接收帧失败
                    qCritical() << "avcodec_receive_frame failed";
                    break;
                }

                // 转换帧格式
                QImage image(m_codecContext->width, m_codecContext->height, QImage::Format_RGB32);
                // 分配 RGB 缓冲区
                uint8_t *rgbBuffer[4] = {image.bits(), nullptr, nullptr, nullptr};
                int rgbLineSize[4] = {static_cast<int>(image.bytesPerLine()), 0, 0, 0};
                // 将 AVFrame 转换为 QImage
                sws_scale(m_swsContext, m_frame->data, m_frame->linesize, 0, m_codecContext->height, rgbBuffer, rgbLineSize);
                // 如果需要垂直翻转图像（ffmpeg 中图像行顺序与 QImage 相反）
                // image = image.mirrored(false, true);
                // 发送图像
                emit videoFrame(image);

                // 如果currentDelay>0，说明解码速度够，还可以加快播放速度
                // 如果currentDelay<0，说明解码速度不够，再想加速需要丢帧播放
                qreal targetDelay = 1000.0 / (m_speed * m_fps);
                qreal currentDelay = targetDelay - timer.elapsed();
                //qDebug() << "currentDelay:" << currentDelay;
                if (currentDelay > 0) {
                    QThread::msleep(currentDelay);
                }
                timer.restart();
            }
            av_frame_unref(m_frame);
            break;
        }
        av_packet_unref(&m_packet);
    }


}
