#include <iostream>
#include <memory>

#include <glibmm/main.h>
#include <gstreamermm.h>
#include <giomm.h>

#include <lightdetection.h>
#include <videorecorder.h>
#include <appsettings.h>

#define LIGHT_DETECTION_FILENAME "/tmp/light-test"
#define LIGHT_DETECTION_SECONDS 5
#define AUDIO_DEVICE "hw:0"
#define VIDEO_BITRATE 1000
#define VIDEO_CAPS "video/x-raw,framerate=30/1"
#define VIDEO_SPEEDPRESET 1
#define VIDEO_DEVICE "/dev/video0"
#define VIDEO_OUTPUT_PATH "/tmp"
#define STREAMING_ENABLE 1
#define STREAMING_PORT 5000
#define INVERT_VALUE 1

ApplicationSettings get_settings_cmdline(int argc, char *argv[])
{
    ApplicationSettings settings;
    settings.input_path = LIGHT_DETECTION_FILENAME;
    settings.output_path = VIDEO_OUTPUT_PATH;
    settings.light_seconds = LIGHT_DETECTION_SECONDS;
    settings.audio_device = AUDIO_DEVICE;
    settings.video_bitrate = VIDEO_BITRATE;
    settings.video_caps = VIDEO_CAPS;
    settings.video_speedpreset = VIDEO_SPEEDPRESET;
    settings.video_device = VIDEO_DEVICE;
    settings.streaming_enable = STREAMING_ENABLE;
    settings.streaming_port = STREAMING_PORT;
    settings.invert_value = INVERT_VALUE;

    Glib::OptionGroup pathesGroup("settings", "application settings");
    Glib::OptionEntry pathDetector;
    pathDetector.set_short_name('i');
    pathDetector.set_long_name("input-path");
    Glib::OptionEntry pathOutput;
    pathOutput.set_short_name('o');
    pathOutput.set_long_name("output-path");
    Glib::OptionEntry lightSeconds;
    lightSeconds.set_short_name('s');
    lightSeconds.set_long_name("light-seconds");
    Glib::OptionEntry audioDevice;
    audioDevice.set_short_name('a');
    audioDevice.set_long_name("alsa-device");
    Glib::OptionEntry videoBitrate;
    videoBitrate.set_short_name('r');
    videoBitrate.set_long_name("video-bitrate");
    Glib::OptionEntry videoCaps;
    videoCaps.set_short_name('c');
    videoCaps.set_long_name("video-caps");
    Glib::OptionEntry videoDevice;
    videoDevice.set_short_name('v');
    videoDevice.set_long_name("v4l-device");
    Glib::OptionEntry videoSpeedPreset;
    videoSpeedPreset.set_short_name('p');
    videoSpeedPreset.set_long_name("video-speed-preset");
    Glib::OptionEntry streamingEnable;
    streamingEnable.set_long_name("enable-streaming");
    Glib::OptionEntry streamingPort;
    streamingPort.set_long_name("streaming-port");
    Glib::OptionEntry invertValue;
    invertValue.set_long_name("invert-value");

    pathesGroup.add_entry_filename(pathDetector, settings.input_path);
    pathesGroup.add_entry_filename(pathOutput, settings.output_path);
    pathesGroup.add_entry(lightSeconds, settings.light_seconds);
    pathesGroup.add_entry(audioDevice, settings.audio_device);
    pathesGroup.add_entry(videoBitrate, settings.video_bitrate);
    pathesGroup.add_entry(videoCaps, settings.video_caps);
    pathesGroup.add_entry(videoDevice, settings.video_device);
    pathesGroup.add_entry(videoSpeedPreset, settings.video_speedpreset);
    pathesGroup.add_entry(streamingEnable, settings.streaming_enable);
    pathesGroup.add_entry(streamingPort, settings.streaming_port);
    pathesGroup.add_entry(invertValue, settings.invert_value);

    Glib::OptionContext context;

    context.set_help_enabled(true);
    context.set_main_group(pathesGroup);

    context.parse(argc, argv);

    return settings;
}

int main(int argc, char *argv[])
{
    Gst::init(argc, argv);
    Gio::init();

    ApplicationSettings app_settings = get_settings_cmdline(argc, argv);

    Glib::RefPtr<Glib::MainLoop> main_loop = Glib::MainLoop::create();

    std::shared_ptr<LightDetection> light(new LightDetection(app_settings.input_path, app_settings.light_seconds,
                                                             app_settings.invert_value));

    std::shared_ptr<VideoRecorder> recorder(new VideoRecorder(light, app_settings));

    if (!recorder->setup_pipeline()) {
        std::cout << "Nie udało się utworzyć pipeline'a" << std::endl;
        return 1;
    }

    recorder->start();

    main_loop->run();

    recorder->stop();

    return 0;
}
