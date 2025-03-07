#ifndef A2DPSTREAMER_H
#define A2DPSTREAMER_H

#include <QObject>

#include <gst/gst.h>

class A2DPStreamer : public QObject
{
    Q_OBJECT
public:
    explicit A2DPStreamer(QObject *parent = nullptr);
    ~A2DPStreamer();

    bool setupPipeline(const QString&);

public slots:
    void play();
    void pause();
    void stop();

signals:
    void connectionStatusChanged(bool);
    // Additional signals for error reporting, etc.

private:
    GstElement* pipeline;
};

#endif // A2DPSTREAMER_H
