#ifndef LIGHTDETECTION_H
#define LIGHTDETECTION_H

#include <string>

#include <glibmm/main.h>

class LightDetection
{
public:
    LightDetection(const std::string &filename, unsigned int seconds, bool invert);
    ~LightDetection();

    typedef sigc::signal<void, bool> type_changed_signal;
    type_changed_signal changed_signal();

    bool initial_value() const;

private:
    bool on_changed(Glib::IOCondition condition);

    void on_timeout();

    bool read_value(int fd);

    int m_fd;
    std::string m_filename;
    unsigned int m_seconds;
    bool m_invert;
    bool m_value;
    bool m_is_waiting;
    type_changed_signal m_changed_signal;
    bool m_initial_value;
};

#endif // LIGHTDETECTION_H
