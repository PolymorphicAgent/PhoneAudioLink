#ifndef A2DPSTREAMER_H
#define A2DPSTREAMER_H

#include <QObject>

#include <gst/gst.h>

class A2DPStreamer : public QObject
{
    Q_OBJECT
public:
    explicit A2DPStreamer(QObject *parent = nullptr);

signals:
};

#endif // A2DPSTREAMER_H
