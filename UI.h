//
// Created by mac on 5/7/16.
//

#ifndef FINALPROJECTBIOMED_UI_H
#define FINALPROJECTBIOMED_UI_H

#include <iostream>
#include <FL/Fl.H>
#include <FL/Fl_window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Progress.H>
#include "Fl_Justify_Input.h"
#include "SliderInput.h"
#include "SliderFloatInput.h"
#include <fstream>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;
typedef void (*callback_function)(void);

class UI {
    public:
        string directoryValue;
        Fl_Justify_Input*  inputSeries;
        Fl_Justify_Input*  outputFile;
        SliderInput *iterationsSlider;
        SliderFloatInput *timeStepSlider;
        SliderFloatInput *conductanceSlider;
        SliderInput *thresholdSlider;
        SliderInput *radiusSlider;
        SliderInput *distanceSlider;
        SliderInput *minSlider;
        Fl_Progress *progress;
        Fl_Button* directoryChooser;
        Fl_Button* but;
        //void (*processFunction)();
        callback_function processFunction;
        void activateSliders();
        void deactivateSliders();
        void updateProgressBar(  float percent);
        static void processCallback( Fl_Widget *w, void *data );
        void updateOutputFile();
        static void open_cb(Fl_Widget *w, void *data);
        void clearOutputFile();
        void setProcessFunction(callback_function);

        string getDirectory();
        string getOutputFile();
        int getIterations();
        float getTimeStep();
        float getConductance();
        int getThreshold();
        int getRadius();
        int getDistance();
        int getMin();




        UI();



};


#endif //FINALPROJECTBIOMED_UI_H
