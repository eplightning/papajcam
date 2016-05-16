#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <glibmm.h>
#include <string>

struct ApplicationSettings {
    std::string input_path;
    std::string output_path;
    Glib::ustring audio_device;
    Glib::ustring video_caps;
    Glib::ustring video_device;
    int light_seconds;
    int video_bitrate;
    int video_speedpreset;
};

#endif // APPSETTINGS_H
