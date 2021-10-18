#ifndef __INC_UNCHAN_COMMON_H
#define __INC_UNCHAN_COMMON_H

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_File_Browser.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Window.H>


#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define REFWRAP(x)      new std::reference_wrapper<typeof(x)>(x)
#define UNWRAP(x, t)    ((std::reference_wrapper<t>*) x)->get()

#endif
