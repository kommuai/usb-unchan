#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>

#include <iomanip>
#include <sstream>

class VisualizerPanel {
    Fl_Pack *group;
    int rlen;
    const char *rbuf;
    int width;

    void render()
    {
        group->clear();
        group->begin();

        int x = 0, rowCount = 1;
        Fl_Pack *row = new Fl_Pack(0, 0, width, 30);
        row->box(FL_NO_BOX);
        row->type(Fl_Pack::HORIZONTAL);
        for (int i = 0; i < rlen; i++) {
            std::ostringstream o;
            o << std::setw(2) << std::hex << std::setfill('0');
            o << (int) rbuf[i];
            if (x + 60 >= width) {
                row->end();
                row = new Fl_Pack(0, 0, width, 30);
                row->box(FL_NO_BOX);
                row->type(Fl_Pack::HORIZONTAL);
                x = 0;
                rowCount++;
            }
            auto b = new Fl_Button(0, 0, 60, 30, "");
            b->copy_label(o.str().c_str());
            b->box(FL_FLAT_BOX);
            x += 60;
        }
        row->end();

        group->size(width, rowCount * 30);

        group->end();
        group->redraw();
    }
public:
    VisualizerPanel() : rlen{0}, rbuf{NULL}, width{490} {}

    void Init()
    {
        auto scroll = new Fl_Scroll(45, 240, width + 10, 110);
        scroll->box(FL_BORDER_FRAME);
        scroll->type(Fl_Scroll::VERTICAL);

        group = new Fl_Pack(45, 240, width, 120);
        group->end();

        scroll->end();
    }

    void Set(const char *b, int len)
    {
        rbuf = b;
        rlen = len;
        render();
    }
};

