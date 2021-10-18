#include <fstream>
#include <unordered_map>
#include <utility>

#include "common.hpp"


class VisualizerPanel;

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

    Fl_File_Browser *schemas;
    VisualizerPanel& Parent;
    std::unordered_map<std::string, Type> TypeMap;
    std::vector<Spec> specs;

public:
    DataRenderer(VisualizerPanel& p);
    static void SchemaEvent(Fl_Widget *w, void *p);
    void LoadDefaultSpec();
    void LoadSpec(const char *fn);
    std::vector<std::pair<std::string, int>> Render(const char *buf, int len);
};


class VisualizerPanel {
    Fl_Pack *group;
    int rlen;
    const char *rbuf;
    int width;
    DataRenderer renderer;
    friend DataRenderer;

    void render();
public:
    VisualizerPanel();
    void Init();
    void Set(const char *b, int len);
};


#ifdef __INTERNAL_UNCHAN_IMPL

DataRenderer::DataRenderer(VisualizerPanel& p)
    : Parent{p},
      TypeMap{
        {"CHAR", Type::CHAR},
        {"HEX", Type::HEX},
        {"SIGNED", Type::SIGNED},
        {"UNSIGNED", Type::UNSIGNED},
      }
{
    LoadDefaultSpec();

    Fl_Group *prev = Fl_Group::current();

    auto w = new Fl_Window(300, 500, "SCHEMAS");
    auto g = new Fl_Pack(0, 0, 300, 500);
    g->type(Fl_Pack::HORIZONTAL);

    schemas = new Fl_File_Browser(0, 0, 300, 0);

    schemas->load("schemas");
    schemas->remove(1);
    schemas->add("--DEFAULT--", NULL);
    schemas->callback(SchemaEvent, REFWRAP(*this));
    schemas->type(FL_MULTI_BROWSER);

    g->end();
    w->end();
    w->show();
    prev->begin();
}

void DataRenderer::SchemaEvent(Fl_Widget *w, void *p)
{
    if (!Fl::event_clicks())
        return;
    Fl_File_Browser *b = (Fl_File_Browser *) w;
    DataRenderer& d = UNWRAP(p, DataRenderer);

    if (b->value() == b->size()) // assume last item is --DEFAULT--
        d.LoadDefaultSpec();
    else
        d.LoadSpec(b->text(b->value()));
    d.Parent.render();
}

void DataRenderer::LoadDefaultSpec()
{
    specs.clear();
    specs.push_back({
            .from = 0,
            .to = -1,
            .width = 1,
            .t = Type::HEX,
    });
}

void DataRenderer::LoadSpec(const char *fn)
{
    std::ifstream f(("schemas/" + std::string(fn)).c_str());
    if (!f)
        return;

    specs.clear();
    while (1) {
        Spec spec;
        std::string type;
        f >> spec.from >> spec.to >> type >> spec.width;
        if (f.eof()) // we didn't get to read all parts, assume failed
            break;
        spec.t = TypeMap[type];
        specs.push_back(spec);
    }
    f.close();
}

std::vector<std::pair<std::string, int>> DataRenderer::Render(
        const char *buf, int len)
{
    std::vector<std::pair<std::string, int>> r;

    if (!buf || len == 0)
        return r;

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


void VisualizerPanel::render()
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
    group->parent()->redraw();
}

VisualizerPanel::VisualizerPanel()
    : rlen{0}, rbuf{NULL}, width{490}, renderer(*this) {}

void VisualizerPanel::Init()
{
    auto scroll = new Fl_Scroll(45, 240, width + 10, 110);
    scroll->box(FL_BORDER_BOX);
    scroll->type(Fl_Scroll::VERTICAL);

    group = new Fl_Pack(45, 240, width, 120);
    group->end();

    scroll->end();
}

void VisualizerPanel::Set(const char *b, int len)
{
    rbuf = b;
    rlen = len;
    render();
}

#endif
