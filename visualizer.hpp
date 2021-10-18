#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>

#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>


class DataRenderer {
    enum Type {
        CHAR,
        HEX,
        SIGNED,
        UNSIGNED,
    };

    struct Spec {
        int from, to, width;
        Type t;
    };

    std::unordered_map<std::string, Type> const TypeMap;
    std::vector<Spec> specs;

public:
    DataRenderer() : TypeMap{
                        {"CHAR", Type::CHAR},
                        {"HEX", Type::HEX},
                        {"SIGNED", Type::SIGNED},
                        {"UNSIGNED", Type::UNSIGNED},
                     }
    {
        LoadDefaultSpec();
    };

    void LoadDefaultSpec()
    {
        specs.clear();
        specs.push_back({
                .from = 0,
                .to = -1,
                .width = 1,
                .t = Type::HEX,
        });
    }

    std::vector<std::pair<std::string, int>> Render(const char *buf, int len)
    {
        std::vector<std::pair<std::string, int>> r;

        int i = 0, ctr = -1;
        for (auto s: specs) {
            ctr++;
            // TODO: sanity check for from and to when loading
            if (s.to == -1) {
                if (ctr == (int) specs.size() - 1)
                    s.to = len;
                else
                    s.to = specs[ctr + 1].from - 1;
            }
            for (;i < s.to;) {
                std::ostringstream o;
                char ibuf[8] = {};
                // TODO: width check when loading
                // TODO: signedness conversion
                for (int j = 0; j < s.width; i++, j++)
                    ibuf[j] = buf[i];

                switch (s.t) {
                case CHAR:
                    o << (char) ibuf[0];
                    break;
                case HEX:
                    o << std::hex << *((unsigned long long*) ibuf);
                    break;
                case SIGNED:
                    o << *((long long*) ibuf);
                    break;
                case UNSIGNED:
                    o << *((unsigned long long*) ibuf);
                    break;
                }
                r.push_back({o.str(), s.width});
            }
        }

        return r;
    }
};


class VisualizerPanel {
    Fl_Pack *group;
    int rlen;
    const char *rbuf;
    int width;
    DataRenderer renderer;

    void render()
    {
        group->clear();
        group->begin();

        int x = 0, rowCount = 1;
        Fl_Pack *row = new Fl_Pack(0, 0, width, 30);
        row->box(FL_NO_BOX);
        row->type(Fl_Pack::HORIZONTAL);

        for (auto s: renderer.Render(rbuf, rlen)) {
            int bwidth = 60 * s.second;
            if (x + bwidth >= width) {
                row->end();
                row = new Fl_Pack(0, 0, width, 30);
                row->box(FL_NO_BOX);
                row->type(Fl_Pack::HORIZONTAL);
                x = 0;
                rowCount++;
            }
            auto b = new Fl_Button(0, 0, bwidth, 30, "");
            b->copy_label(s.first.c_str());
            b->box(FL_FLAT_BOX);
            x += bwidth;
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

