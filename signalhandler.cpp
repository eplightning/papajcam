#include "signalhandler.h"

#include <sys/signalfd.h>
#include <unistd.h>
#include <iostream>

SignalHandler::SignalHandler(Glib::RefPtr<Glib::MainLoop> loop)
{
    m_loop = loop;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGTERM);

    m_fd = signalfd(-1, &mask, 0);

    m_connection = Glib::signal_io().connect(sigc::mem_fun(this, &SignalHandler::on_signal), m_fd, Glib::IOCondition::IO_IN);
}

SignalHandler::~SignalHandler()
{
    m_connection.disconnect();
    close(m_fd);
}

bool SignalHandler::on_signal(Glib::IOCondition condition)
{
    struct signalfd_siginfo fdsi;
    read(m_fd, &fdsi, sizeof(struct signalfd_siginfo));

    m_loop->quit();
}
