#ifndef VIDEORECORDER_H
#define VIDEORECORDER_H

#include <memory>

#include <lightdetection.h>

#include <gstreamermm.h>

#include <gstreamermm/valve.h>

#include <appsettings.h>

class VideoRecorder
{
public:
    VideoRecorder(std::shared_ptr<LightDetection>& light, const ApplicationSettings &settings);

    bool setup_pipeline();
    void start();
    void stop();

private:
    void on_light(bool value);
    bool on_bus_message(const Glib::RefPtr<Gst::Bus>&,
                        const Glib::RefPtr<Gst::Message>& message);
    void start_recording();

    std::string generate_filename();
    Glib::RefPtr<Gst::Queue> setup_queue();

    const ApplicationSettings &m_settings;
    std::shared_ptr<LightDetection> m_light;

    bool m_recording;
    bool m_waiting_for_eos;
    bool m_start_after_eos;

    Glib::RefPtr<Gst::Pipeline> m_pipeline;

    Glib::RefPtr<Gst::Valve> m_valve_video;
    Glib::RefPtr<Gst::Valve> m_valve_audio;
    Glib::RefPtr<Gst::FileSink> m_filesink;

    Glib::RefPtr<Gst::Queue> m_queue_video;
    Glib::RefPtr<Gst::Queue> m_queue_audio;
};

#endif // VIDEORECORDER_H
