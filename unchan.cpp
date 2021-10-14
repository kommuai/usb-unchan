#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Choice.H>
#include <libusb-1.0/libusb.h>

#include <iomanip>
#include <string>
#include <sstream>


int main(int argc, char **argv)
{
    libusb_init(NULL);

    auto w = new Fl_Window(600, 400, "USB Un-chan!");

    auto devList = new Fl_Choice(100, 10, 100, 30, "device");

    libusb_device **list;
    ssize_t cnt = libusb_get_device_list(NULL, &list);
    for (ssize_t i = 0; i < cnt; i++) {
        unsigned char buf[128] = {};
        libusb_device_descriptor desc;
        libusb_get_device_descriptor(list[i], &desc);
        libusb_device_handle *h;
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

        devList->add(o.str().c_str());
    }
    libusb_free_device_list(list, 1);


    w->end();
    w->show(argc, argv);

    int ret = Fl::run();

    libusb_exit(NULL);

    return ret;
}
