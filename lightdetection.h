#ifndef LIGHTDETECTION_H
#define LIGHTDETECTION_H

#include <string>

#include <glibmm/main.h>
#include <giomm/filemonitor.h>
#include <giomm/file.h>

class LightDetection
{
public:
    LightDetection(const std::string &filename, unsigned int seconds, bool invert);

    typedef sigc::signal<void, bool> type_changed_signal;
    type_changed_signal changed_signal();

    bool initial_value() const;

private:
    void on_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file,
                    Gio::FileMonitorEvent event_type);

    void on_timeout();

    bool read_value(const Glib::RefPtr<Gio::File> &file);

    Glib::RefPtr<Gio::FileMonitor> m_monitor;
    Glib::RefPtr<Gio::File> m_file;
    std::string m_filename;
    unsigned int m_seconds;
    bool m_invert;
    bool m_value;
    bool m_is_waiting;
    type_changed_signal m_changed_signal;
    bool m_initial_value;
};

#endif // LIGHTDETECTION_H
