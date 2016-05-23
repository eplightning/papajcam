#include "lightdetection.h"

#include <iostream>
#include <fcntl.h>
#include <unistd.h>

#include <glibmm/main.h>

LightDetection::LightDetection(const std::string &filename, unsigned int seconds, bool invert)
{
    m_filename = filename;

    m_fd = open(filename.c_str(), O_RDONLY);

    if (m_fd < 0) {
        std::cout << "[LightDetection] Nie mozna otworzyc pliku wejsciowego";
        throw new std::exception();
    }

    m_seconds = seconds;
    m_is_waiting = false;
    m_invert = invert;
    m_initial_value = read_value(m_fd);
    m_value = m_initial_value;

    Glib::signal_io().connect(sigc::mem_fun(this, &LightDetection::on_changed), m_fd, Glib::IO_PRI, Glib::PRIORITY_DEFAULT);
}

LightDetection::~LightDetection()
{
    if (m_fd >= 0)
        close(m_fd);
}

LightDetection::type_changed_signal LightDetection::changed_signal()
{
    return m_changed_signal;
}

bool LightDetection::on_changed(Glib::IOCondition condition)
{
    if (condition != Glib::IO_PRI)
        return true;

    if (m_is_waiting)
        return true;

    bool val = read_value(m_fd);

    if (val == m_value)
        return true;

    m_value = val;
    m_is_waiting = true;

    Glib::signal_timeout().connect_seconds_once(sigc::mem_fun(this, &LightDetection::on_timeout), m_seconds);

    return true;
}

void LightDetection::on_timeout()
{
    if (!m_is_waiting) {
        std::cout << "[LightDetection] m_is_waiting == false w on_timeout ?!" << std::endl;
        return;
    }

    if (m_value == read_value(m_fd)) {
        std::cout << "[LightDetection] Wartosc zostala zmieniona pomyslnie" << std::endl;
        m_changed_signal.emit(m_value);
    } else {
        std::cout << "[LightDetection] Zmiana wartości zignorowana" << std::endl;
    }

    m_is_waiting = false;
}

bool LightDetection::read_value(int file)
{
    lseek(file, 0, SEEK_SET);

    char buf[1];

    if (read(file, buf, 1) != 1) {
        std::cout << "[LightDetection] Nie można bylo odczytac bajtu z pliku, uznajemy że wartosc 0" << std::endl;
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
