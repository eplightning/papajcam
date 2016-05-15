#ifndef VIDEORECORDER_H
#define VIDEORECORDER_H

#include <memory>

#include <lightdetection.h>

#include <gstreamermm.h>

#include <gstreamermm/valve.h>

struct VideoRecorderSettings {
    const char *video_cads;
    int speed_preset;
    int bitrate;
    const char *video_device;
    const char *audio_device;
};

class VideoRecorder
{
public:
    VideoRecorder(std::shared_ptr<LightDetection>& light, const std::string &directory, const VideoRecorderSettings &settings);

    bool setup_pipeline();
    void start();
    void stop();

private:
    void on_light(bool value);
    bool on_bus_message(const Glib::RefPtr<Gst::Bus>&,
                        const Glib::RefPtr<Gst::Message>& message);
    void start_recording();

    bool m_recording;
    bool m_waiting_for_eos;
    bool m_start_after_eos;

    std::string generate_filename();
    Glib::RefPtr<Gst::Queue> setup_queue();

    Glib::RefPtr<Gst::Pipeline> m_pipeline;
    std::shared_ptr<LightDetection> m_light;
    std::string m_directory;
    const VideoRecorderSettings &m_settings;

    Glib::RefPtr<Gst::Valve> m_valve_video;
    Glib::RefPtr<Gst::Valve> m_valve_audio;
    Glib::RefPtr<Gst::FileSink> m_filesink;

    Glib::RefPtr<Gst::Queue> m_queue_video;
    Glib::RefPtr<Gst::Queue> m_queue_audio;
};

#endif // VIDEORECORDER_H
