#include <iostream>
#include <memory>

#include <glibmm/main.h>
#include <gstreamermm.h>
#include <giomm.h>

#include <lightdetection.h>
#include <videorecorder.h>

#define LIGHT_DETECTION_FILENAME "/tmp/light-test"
#define LIGHT_DETECTION_SECONDS 5
#define AUDIO_DEVICE "hw:0"
#define VIDEO_BITRATE 1000
#define VIDEO_CAPS "video/x-raw,framerate=30/1"
#define VIDEO_SPEEDPRESET 1
#define VIDEO_DEVICE "/dev/video0"

int main(int argc, char *argv[])
{
    Gst::init(argc, argv);
    Gio::init();

    Glib::RefPtr<Glib::MainLoop> main_loop = Glib::MainLoop::create();

    std::shared_ptr<LightDetection> light(new LightDetection(LIGHT_DETECTION_FILENAME, LIGHT_DETECTION_SECONDS));

    VideoRecorderSettings settings;
    settings.audio_device = AUDIO_DEVICE;
    settings.bitrate = VIDEO_BITRATE;
    settings.video_cads = VIDEO_CAPS;
    settings.speed_preset = VIDEO_SPEEDPRESET;
    settings.video_device = VIDEO_DEVICE;

    std::shared_ptr<VideoRecorder> recorder(new VideoRecorder(light, "/tmp", settings));

    if (!recorder->setup_pipeline()) {
        std::cout << "Nie udało się utworzyć pipeline'a" << std::endl;
        return 1;
    }

    recorder->start();

    main_loop->run();

    recorder->stop();

    return 0;
}
