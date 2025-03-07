#include "a2dpstreamer.h"
#include "qdebug.h"

A2DPStreamer::A2DPStreamer(QObject* parent)
    : QObject(parent), pipeline(nullptr)
{
    gst_init(nullptr, nullptr);
}

A2DPStreamer::~A2DPStreamer() {
    if (pipeline)
        gst_object_unref(pipeline);
}

// Initialize the pipeline with elements suited for A2DP.
// Note: The exact pipeline will depend on your target platform
// and available plugins. For instance, on Linux you might use
// the "bluez_a2dp_sink" element with BlueZ.
bool A2DPStreamer::setupPipeline(const QString& deviceAddress) {
    // Build your pipeline string using the deviceAddress.
    // This is a simplified example:
    QString pipelineStr = QString("audiotestsrc ! audioconvert ! audioresample ! "
                                  "wasapisink device=%1")
                              .arg(deviceAddress);
    GError* error = nullptr;
    pipeline = gst_parse_launch(pipelineStr.toUtf8().constData(), &error);
    if (!pipeline) {
        qWarning() << "Failed to create pipeline:" << error->message;
        g_error_free(error);
        return false;
    }
    return true;
}

void A2DPStreamer::play() {
    if (pipeline)
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void A2DPStreamer::pause() {
    if (pipeline)
        gst_element_set_state(pipeline, GST_STATE_PAUSED);
}

void A2DPStreamer::stop() {
    if (pipeline)
        gst_element_set_state(pipeline, GST_STATE_NULL);
}
