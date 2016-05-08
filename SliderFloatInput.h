//
// Created by mac on 5/7/16.
//

#ifndef FINALPROJECTBIOMED_SLIDERFLOATINPUT_H
#define FINALPROJECTBIOMED_SLIDERFLOATINPUT_H

#include <FL/Fl.H>
#include <FL/Fl_window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Progress.H>


class SliderFloatInput : public Fl_Group {
    Fl_Float_Input *input;
    Fl_Slider    *slider;

    // CALLBACK HANDLERS
    //    These 'attach' the input and slider's values together.
    //
    void Slider_CB2() {
        static int recurse = 0;
        if ( recurse ) {
            return;
        } else {
            recurse = 1;
            char s[80];
            sprintf(s, "%.2f", (float)(slider->value() ));
            // fprintf(stderr, "SPRINTF(%d) -> '%s'\n", (int)(slider->value()+.5), s);
            input->value(s);          // pass slider's value to input
            recurse = 0;
        }
    }

    static void Slider_CB(Fl_Widget *w, void *data) {



        ((SliderFloatInput*)data)->Slider_CB2();
    }

    void Input_CB2() {
        static int recurse = 0;
        if ( recurse ) {
            return;
        } else {
            recurse = 1;
            int val = 0;
            if ( sscanf(input->value(), "%.2f", &val) != 1 ) {
                val = 0;
            }
            // fprintf(stderr, "SCANF('%s') -> %d\n", input->value(), val);
            slider->value(val);         // pass input's value to slider


            recurse = 0;
        }
    }
    static void Input_CB(Fl_Widget *w, void *data) {



        ((SliderFloatInput*)data)->Input_CB2();
    }

public:
    // CTOR
    SliderFloatInput(int x, int y, int w, int h, const char *l=0) : Fl_Group(x,y,w,h,l) {
        int in_w = 40;
        input  = new Fl_Float_Input(x, y, in_w, h);
        input->callback(Input_CB, (void*)this);
        input->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);

        slider = new Fl_Slider(x+in_w, y, w-in_w, h);
        slider->type(1);
        slider->callback(Slider_CB, (void*)this);

        bounds(1, 10);     // some usable default
        value(5);          // some usable default
        end();             // close the group
    }

    // MINIMAL ACCESSORS --  Add your own as needed
    float  value() const    { return((float)(slider->value())); }
    void value(float val)   { slider->value(val); Slider_CB2(); }
    void minumum(float val) { slider->minimum(val); }
    int  minumum() const  { return((float)slider->minimum()); }
    void maximum(float val) { slider->maximum(val); }
    int  maximum() const  { return((int)slider->maximum()); }
    void bounds(float low, float high) { slider->bounds(low, high); }
};

#endif //FINALPROJECTBIOMED_SLIDERFLOATINPUT_H
