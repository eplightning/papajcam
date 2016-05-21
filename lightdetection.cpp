#include "lightdetection.h"

#include <iostream>

#include <giomm/filemonitor.h>
#include <giomm/file.h>
#include <glibmm/main.h>

LightDetection::LightDetection(const std::string &filename, unsigned int seconds, bool invert)
{
    m_filename = filename;
    m_seconds = seconds;
    m_file = Gio::File::create_for_path(filename);
    m_monitor = m_file->monitor_file();
    m_is_waiting = false;
    m_invert = invert;
    m_initial_value = read_value(m_file);
    m_value = m_initial_value;

    m_monitor->signal_changed().connect(sigc::mem_fun(this, &LightDetection::on_changed));
}

LightDetection::type_changed_signal LightDetection::changed_signal()
{
    return m_changed_signal;
}

void LightDetection::on_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file,
                                Gio::FileMonitorEvent event_type)
{
    if (event_type != Gio::FileMonitorEvent::FILE_MONITOR_EVENT_CHANGED)
        return;

    if (m_is_waiting)
        return;

    bool val = read_value(file);

    if (val == m_value)
        return;

    m_value = val;
    m_is_waiting = true;

    Glib::signal_timeout().connect_seconds_once(sigc::mem_fun(this, &LightDetection::on_timeout), m_seconds);
}

void LightDetection::on_timeout()
{
    if (!m_is_waiting) {
        std::cout << "[LightDetection] m_is_waiting == false w on_timeout ?!" << std::endl;
        return;
    }

    if (m_value == read_value(m_file)) {
        std::cout << "[LightDetection] Wartość została zmieniona pomyślnie" << std::endl;
        m_changed_signal.emit(m_value);
    } else {
        std::cout << "[LightDetection] Zmiana wartości zignorowana" << std::endl;
    }

    m_is_waiting = false;
}

bool LightDetection::read_value(const Glib::RefPtr<Gio::File> &file)
{
    Glib::RefPtr<Gio::FileInputStream> stream = file->read();

    char buf[1];

    if (stream->read(buf, 1) != 1) {
        std::cout << "[LightDetection] Nie można było odczytać bajtu z pliku, uznajemy że wartość 0" << std::endl;
        return false;
    }

    if (m_invert)
        return buf[0] == '1' ? false : true;

    return buf[0] == '1' ? true : false;
}

bool LightDetection::initial_value() const
{
    return m_initial_value;
}
