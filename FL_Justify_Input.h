/*!
 * @brief Used for displaying justified text
 * @file
 * */
#ifndef FINALPROJECTBIOMED_FL_JUSTIFY_INPUT_H
#define FINALPROJECTBIOMED_FL_JUSTIFY_INPUT_H


#include <FL/Fl.H>
#include <FL/Fl_window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Progress.H>


class Fl_Justify_Input : public Fl_Group {
    Fl_Input *inp;
    Fl_Box   *box;
public:
    //! Constructor
    Fl_Justify_Input(int X,int Y,int W,int H,const char *L=0):Fl_Group(X,Y,W,H,L) {
        Fl_Group::box(FL_NO_BOX);
        box = new Fl_Box(X,Y,W,H);
        box->color(FL_WHITE);
        box->box(FL_DOWN_BOX);
        inp = new Fl_Input(X,Y,W,H);
        inp->hide();
        inp->color(FL_WHITE);
        inp->readonly(1);

        end();
    }
    //! Set text justification
    void justify(Fl_Align val) {
        box->align(val|FL_ALIGN_INSIDE);
    }
    //! Returns text justification
    Fl_Align justify() const {
        return box->align();
    }
    //! Sets the text value
    void value(const char *val) {
        box->copy_label(val);
        inp->value(val);
    }
    //! Gets the text value
    const char *value() const {
        return inp->value();
    }
    int handle(int e) {
        switch (e) {
            case FL_PUSH:
            case FL_FOCUS:
                if (!inp->visible()) {
                    // Make input widget 'appear' in place of the box
                    box->hide();
                    inp->show();
                    inp->value(box->label());
                    redraw();
                }
                break;
            case FL_UNFOCUS:
                if (inp->visible()) {
                    // Replace input widget with justified box
                    box->show();
                    inp->hide();
                    box->label(inp->value());
                    redraw();
                }
                break;
            default:
                break;
        }
        return(Fl_Group::handle(e));
    }
};



#endif //FINALPROJECTBIOMED_FL_JUSTIFY_INPUT_H
