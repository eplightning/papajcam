#include "videorecorder.h"

#include <iostream>
#include <string>

#include <glibmm/datetime.h>
#include <glibmm/ustring.h>
#include <gstreamermm/valve.h>
#include <gstreamermm/tee.h>

using namespace Gst;


VideoRecorder::VideoRecorder(std::shared_ptr<LightDetection> &light,
                             const ApplicationSettings &settings) :
    m_settings(settings), m_light(light)
{
    m_recording = false;
    m_waiting_for_eos = false;
    m_start_after_eos = false;
}

std::string VideoRecorder::generate_filename()
{
    std::string path(m_settings.output_path);

    if (path.back() != '/' && path.back() != '\\')
        path += '/';

    Glib::DateTime dt = Glib::DateTime::create_now_local();

    path.append(dt.format("%F_%T"));

    path.append(".mkv");

    return path;
}

bool VideoRecorder::setup_pipeline()
{
    m_pipeline = Pipeline::create();

    // główne elementy
    Glib::RefPtr<Element> videoSrc = ElementFactory::create_element("v4l2src");
    Glib::RefPtr<Element> videoConvert = ElementFactory::create_element("videoconvert");
    Glib::RefPtr<Element> videoEncoder = ElementFactory::create_element("x264enc");
    Glib::RefPtr<AlsaSrc> audioSrc = AlsaSrc::create();
    Glib::RefPtr<Element> audioConvert = ElementFactory::create_element("audioconvert");
    Glib::RefPtr<Element> audioEncoder = ElementFactory::create_element("faac");
    Glib::RefPtr<Element> muxer = ElementFactory::create_element("matroskamux");
    Glib::RefPtr<FileSink> fileSink = FileSink::create();

    // sprawdzamy czy się udało
    if (!videoSrc || !videoConvert || !videoEncoder || !audioSrc || !audioConvert
            || !audioEncoder || !muxer || !fileSink) {
        std::cout << "[VideoRecorder] Podstawowe elementy gstreamer'a nie mogły zostać utworzone";
        return false;
    }

    // dodatkowe elementy, kolejki (2x bezpośrednio po wejściu i 2x przed muxerem)
    Glib::RefPtr<Queue> queue1 = setup_queue();
    Glib::RefPtr<Queue> queue2 = setup_queue();
    Glib::RefPtr<Queue> queue3 = setup_queue();
    Glib::RefPtr<Queue> queue4 = setup_queue();

    // dodatkowe elementy, zawory
    Glib::RefPtr<Valve> valve1 = Valve::create();
    Glib::RefPtr<Valve> valve2 = Valve::create();
    Glib::RefPtr<Tee> tee1 = Tee::create();

    // ustawienia elementów
    videoSrc->property("do-timestamp", true)
            ->property("device", Glib::ustring(m_settings.video_device));

    videoEncoder->property("pass", 0)->property("bitrate", m_settings.video_bitrate)
                ->property("speed-preset", m_settings.video_speedpreset)
                ->property("byte-stream", true);

    //audioSrc->property_device().set_value(m_settings.audio_device);
    //audioSrc->property_do_timestamp().set_value(true);

    // ograniczenia
    Glib::RefPtr<Caps> videoCaps = Caps::create_from_string(m_settings.video_caps);
    Glib::RefPtr<Caps> audioCaps = Caps::create_any();

    // wrzucamy wszystko do pipeline'a
    m_pipeline->add(videoSrc)->add(videoConvert)->add(videoEncoder)->add(muxer)->add(fileSink)
              ->add(queue1)->add(queue2)->add(valve1)->add(tee1);

    //m_pipeline->add(audioSrc)->add(audioConvert)->add(audioEncoder)->add(queue3)->add(queue4)
    //          ->add(valve2);

    // łączenie przebiegu video
    videoSrc->link(valve1, videoCaps)->link(queue1)->link(videoConvert)->link(videoEncoder)
            ->link(queue2)->link(tee1);

    // łączenie przebiegu audio
    //audioSrc->link(valve2, audioCaps)->link(queue3)->link(audioConvert)->link(audioEncoder)
    //        ->link(queue4);

    // muxer
    tee1->get_request_pad("src_0")->link(muxer->get_request_pad("video_0"));
    //queue2->get_static_pad("src")->link(muxer->get_request_pad("video_0"));
    //queue4->get_static_pad("src")->link(muxer->get_request_pad("audio_0"));
    muxer->link(fileSink);

    /*if (m_settings.streaming_enable) {
        Glib::RefPtr<Element> payloader = ElementFactory::create_element("rtph264pay");
        Glib::RefPtr<Element> bin = ElementFactory::create_element("rtpbin");
        Glib::RefPtr<Element> udp = ElementFactory::create_element("udpsink");
        Glib::RefPtr<Element> udp2 = ElementFactory::create_element("udpsink");

        if (!payloader || !bin || !udp || !udp2) {
            std::cout << "[VideoRecorder] Podstawowe elementy gstreamer'a dot. strumieniowania nie mogły zostać utworzone";
            return false;
        }

        m_pipeline->add(payloader)->add(bin)->add(udp)->add(udp2);
        //m_pipeline->add(udp);

        //udp->set_property("sync", false);
        udp->set_property("async", false);
        udp->set_property("port", 81);
        //udp2->set_property("sync", false);
        udp2->set_property("async", false);
        udp2->set_property("port", m_settings.streaming_port + 1);

        //tee1->get_request_pad("src_1")->link(udp->get_static_pad("sink"));
        tee1->get_request_pad("src_1")->link(payloader->get_static_pad("sink"));
        payloader->get_static_pad("src")->link(bin->get_request_pad("send_rtp_sink_0"));
        bin->get_static_pad("send_rtp_src_0")->link(udp->get_static_pad("sink"));
        bin->get_request_pad("send_rtcp_src_0")->link(udp2->get_static_pad("sink"));
    }*/

    m_valve_video = valve1;
    m_queue_video = queue1;
    //m_valve_audio = valve2;
    //m_queue_audio = queue3;
    m_filesink = fileSink;

    return true;
}

void VideoRecorder::start()
{
    if (m_light->initial_value())
        start_recording();

    m_light->changed_signal().connect(sigc::mem_fun(this, &VideoRecorder::on_light));
    m_pipeline->get_bus()->add_watch(sigc::mem_fun(this, &VideoRecorder::on_bus_message));
}

void VideoRecorder::stop()
{
    m_pipeline->set_state(State::STATE_NULL);
}

void VideoRecorder::on_light(bool value)
{
    if (value) {
        if (m_recording)
            return;

        if (m_waiting_for_eos) {
            m_start_after_eos = true;
            return;
        }

        start_recording();
    } else {
        if (!m_recording) {
            if (m_start_after_eos)
                m_start_after_eos = false;

            return;
        }

        std::cout << "[VideoRecorder] Konczenie nagrywania video ..." << std::endl;

        // zakręcami korki od źródeł
        m_valve_video->property_drop().set_value(true);

        // wysyłamy EOS żeby zamknąć plik
        m_queue_video->send_event(EventEos::create());

        m_waiting_for_eos = true;
        m_start_after_eos = false;
        m_recording = false;
    }
}

bool VideoRecorder::on_bus_message(const Glib::RefPtr<Bus> &, const Glib::RefPtr<Message> &message)
{
    if (message->get_message_type() == MESSAGE_EOS) {
        if (!m_waiting_for_eos)
            return true;

        std::cout << "[VideoRecorder] Nagrywanie video zakonczone ..." << std::endl;

        m_waiting_for_eos = false;

        if (m_start_after_eos) {
            m_start_after_eos = false;
            start_recording();
        } else {
            m_pipeline->set_state(State::STATE_NULL);
        }
    }

    return true;
}

void VideoRecorder::start_recording()
{
    std::cout << "[VideoRecorder] Start nagrywania video ..." << std::endl;
    m_valve_video->property_drop().set_value(false);
    m_filesink->property_location().set_value(Glib::ustring(generate_filename()));

    m_recording = true;
    m_pipeline->set_state(State::STATE_PLAYING);
}

Glib::RefPtr<Queue> VideoRecorder::setup_queue()
{
    Glib::RefPtr<Queue> q = Queue::create();

    q->property_max_size_buffers().set_value(0);
    q->property_max_size_bytes().set_value(0);
    q->property_max_size_time().set_value(0);

    return q;
}
