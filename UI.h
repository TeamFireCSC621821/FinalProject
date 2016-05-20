/*!
 * @brief This file holds the UI logic and can get input from user and pass to the processing chain
 * @file
 * */
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

//! Class used for containing the UI for user input
class UI {
    public:
        string directoryValue; //! Holds the user selected directory
        Fl_Justify_Input*  inputSeries; //!Displays directory of dicom slides
        Fl_Justify_Input*  outputFile; //!For displaying output file
        SliderInput *iterationsSlider; //! int slider for selecting number of iterations on smoothing filter
        SliderFloatInput *timeStepSlider; //! float slider for time step on smoothing filter
        SliderFloatInput *conductanceSlider; //! float slider for conductance parameter on smoothing filter
        SliderInput *thresholdSlider; //! int slider for both threshold filters
        SliderInput *radiusSlider; //! int slider for radius on closing filter
        SliderInput *distanceSlider;
        SliderInput *minSlider; //! min voxel size for connected components
        Fl_Progress *progress; //! to display progress
        Fl_Button* directoryChooser; //! directory chooser
        Fl_Button* but; //!Process button
        //void (*processFunction)();
        callback_function processFunction; //! Callback to initialize the process function
        void activateSliders(); //! Activate the sliders
        void deactivateSliders(); //! Deactivate sliders (While processing)
        void updateProgressBar(  float percent); //!For updating prgress barr status
        static void processCallback( Fl_Widget *w, void *data ); //! callback to process
        void updateOutputFile(); //! Update the name of the output file with parameters
        static void open_cb(Fl_Widget *w, void *data); //! opens the directory chooser
        void clearOutputFile(); //! clears the output file display
        void setProcessFunction(callback_function); //! Sets the pointer of the function that gets called on processess

        string getDirectory(); //! Gets name of directory chosen
        string getOutputFile(); //! Gets name of output file
        int getIterations(); //! Gets number of iterations from UI
        float getTimeStep(); //! Gets time step from UI
        float getConductance(); //! Gets conductance parameter from UI
        int getThreshold(); //! Gets Threshold from UI
        int getRadius(); //! Gets Radius from UI
        int getMin(); //! Gets min Component size from UI


        UI();



};


#endif //FINALPROJECTBIOMED_UI_H
