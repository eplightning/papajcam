#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include <glibmm/main.h>

class SignalHandler
{
public:
    SignalHandler(Glib::RefPtr<Glib::MainLoop> loop);
    ~SignalHandler();

private:
    bool on_signal(Glib::IOCondition condition);

    int m_fd;
    Glib::RefPtr<Glib::MainLoop> m_loop;
    sigc::connection m_connection;
};

#endif // SIGNALHANDLER_H
