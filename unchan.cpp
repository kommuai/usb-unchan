#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Window.H>
#include <libusb-1.0/libusb.h>

#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "visualizer.hpp"

#define REFWRAP(x)      new std::reference_wrapper<typeof(x)>(x)
#define UNWRAP(x, t)    ((std::reference_wrapper<t>*) x)->get()

/*
 * Development Note
 *
 *  - Fl_Widget constructor does not copy label, for dynamic string use
 *    copy_label().
 *
 */


class DevicePanel {
    Fl_Group *group;
    Fl_Int_Input *txtReq, *txtLen, *txtVal, *txtIdx;
    libusb_device_handle *dev;
    unsigned char rbuf[256];

    VisualizerPanel vizPanel;
public:
    DevicePanel(libusb_device_handle *h) : dev{h}
    {
        group = new Fl_Group(40, 60, 520, 300);
        group->box(FL_BORDER_BOX);
        group->color(fl_lighter(fl_lighter(fl_lighter(FL_BLUE))));

        int left = group->x() + 10 + 120;
        txtReq = new Fl_Int_Input(left, group->y() + 10,
                                  100, 30,
                                  "bRequest (HEX):");
        txtVal = new Fl_Int_Input(left, txtReq->y() + txtReq->h() + 10,
                                  100, 30,
                                  "wValue:");
        txtIdx = new Fl_Int_Input(left, txtVal->y() + txtVal->h() + 10,
                                  100, 30,
                                  "wIndex:");
        txtLen = new Fl_Int_Input(left, txtIdx->y() + txtIdx->h() + 10,
                                  100, 30,
                                  "wLength:");

        txtVal->value("0");
        txtIdx->value("0");

        auto btnSend = new Fl_Button(txtLen->x() + txtLen->w() + 10, txtLen->y(),
                                     100, 30,
                                     "send");
        btnSend->callback(ButtonSend, REFWRAP(*this));

        vizPanel.Init();

        group->end();
    }

    ~DevicePanel()
    {
        libusb_close(dev);
    }

    static void ButtonSend(Fl_Widget *w, void *p)
    {
        DevicePanel& d = UNWRAP(p, DevicePanel);
        unsigned int bRequest, wValue, wIndex, wLength;
        std::istringstream i(d.txtReq->value());
        i >> std::hex >> bRequest;

        i = std::istringstream(d.txtVal->value());
        i >> std::dec >> wValue;

        i = std::istringstream(d.txtIdx->value());
        i >> std::dec >> wIndex;

        i = std::istringstream(d.txtLen->value());
        i >> std::dec >> wLength;


        int rx = libusb_control_transfer(d.dev,
                                         LIBUSB_ENDPOINT_IN |
                                            LIBUSB_REQUEST_TYPE_VENDOR |
                                            LIBUSB_RECIPIENT_DEVICE,
                                         (uint8_t) bRequest,
                                         wValue,
                                         wIndex,
                                         d.rbuf,
                                         (uint16_t) wLength,
                                         200);

        d.vizPanel.Set((const char *) d.rbuf, rx);
    }
};

class DeviceList {
    Fl_Choice *combo;
    Fl_Button *button;
    libusb_device **list;
public:
    DeviceList() : combo{NULL}, list{NULL}
    {
        combo = new Fl_Choice(100, 10, 105, 30, "device:");
        button = new Fl_Button(combo->x() + combo->w() + 10, combo->y(),
                               60, combo->h(),
                               "open");
        button->callback(ButtonOpen, REFWRAP(*this));
    }

    ~DeviceList()
    {
        if (list)
            libusb_free_device_list(list, 1);
    }

    void Load()
    {
        if (list)
            libusb_free_device_list(list, 1);
        combo->clear();

        ssize_t cnt = libusb_get_device_list(NULL, &list);
        for (ssize_t i = 0; i < cnt; i++) {
            unsigned char buf[128] = {};
            libusb_device_descriptor desc;
            libusb_get_device_descriptor(list[i], &desc);
            libusb_device_handle *h = NULL;
            libusb_open(list[i], &h);
            if (h) {
                libusb_get_string_descriptor_ascii(h, desc.iProduct, buf, 128);
                libusb_close(h);
            }

            std::ostringstream o;
            o << std::hex << std::setfill('0');
            o << std::setw(4) << desc.idVendor;
            o << ':';
            o << std::setw(4) << desc.idProduct;
            o << ' ' << buf << std::endl;

            combo->add(o.str().c_str());
        }
    }

    static void ButtonOpen(Fl_Widget *w, void *p)
    {
        DeviceList& d = UNWRAP(p, DeviceList);
        Fl_Choice *c = d.combo;

        if (c->value() == -1)
            return;

        libusb_device_descriptor desc;
        libusb_get_device_descriptor(d.list[c->value()], &desc);
        libusb_device_handle *h;
        libusb_open(d.list[c->value()], &h);
        libusb_claim_interface(h, 0);

        d.combo->parent()->begin();

        if (h)
            new DevicePanel(h);

        d.combo->parent()->end();
        d.combo->parent()->redraw();
    }
};

int main(int argc, char **argv)
{
    libusb_init(NULL);

    auto w = new Fl_Window(600, 400, "USB Un-chan!");

    auto devList = DeviceList();
    devList.Load();

    w->end();
    w->show(argc, argv);

    int ret = Fl::run();

    libusb_exit(NULL);

    return ret;
}
