//
// Created by mac on 5/7/16.
//

#include "UI.h"


using namespace std;



UI::UI(){

    int widgetHeight = 25;
    int widgetWidth = 400;
    int leftMargin = 25;
    int topOffset = 25;
    int ySpacing = 55;
    int yOffsets[12];
    for (int i = 0; i < 11; i++){
        yOffsets[i] = topOffset + (i*ySpacing);
    }

    Fl_Window* win= new Fl_Window( 450,650, "SEGMENTATION PROCESSOR" );
    win->begin();

    directoryChooser = new Fl_Button( leftMargin, yOffsets[0], widgetWidth,widgetHeight, "Choose Directory" );

    inputSeries = new Fl_Justify_Input(leftMargin ,yOffsets[1], widgetWidth , widgetHeight, "Input Series");
    inputSeries->justify(FL_ALIGN_CENTER);

    outputFile = new Fl_Justify_Input(leftMargin ,yOffsets[2], widgetWidth , widgetHeight, "Output File");
    outputFile->justify(FL_ALIGN_CENTER);

    iterationsSlider = new SliderInput(leftMargin, yOffsets[3] ,widgetWidth,widgetHeight, "Iterations");
    iterationsSlider->bounds(1,10);       // set min/max for slider
    iterationsSlider->value(2);           // set initial value


    timeStepSlider = new SliderFloatInput(leftMargin,yOffsets[4],widgetWidth,widgetHeight, "TimeStep");
    timeStepSlider->bounds(0.0f ,0.25f);       // set min/max for slider
    timeStepSlider->value(0.01f);           // set initial value


    conductanceSlider = new SliderFloatInput(leftMargin,yOffsets[5],widgetWidth,widgetHeight, "Conductance Parameter");
    conductanceSlider->bounds(0.0f ,0.5f);       // set min/max for slider
    conductanceSlider->value(0.3f);           // set initial value


    thresholdSlider = new SliderInput(leftMargin,yOffsets[6],widgetWidth,widgetHeight, "Threshold");
    thresholdSlider->bounds(0,500);       // set min/max for slider
    thresholdSlider->value(250);           // set initial value


    radiusSlider = new SliderInput(leftMargin,yOffsets[7],widgetWidth,widgetHeight, "Closing Filter Radius");
    radiusSlider->bounds(1,10);       // set min/max for slider
    radiusSlider->value(4);           // set initial value
/*
    distanceSlider = new SliderInput(leftMargin,yOffsets[8],widgetWidth,widgetHeight, "Distance Threshold");
    distanceSlider->bounds(1,30);       // set min/max for slider
    distanceSlider->value(2);           // set initial value
*/
    minSlider = new SliderInput(leftMargin,yOffsets[8],widgetWidth,widgetHeight, "Min Size");
    minSlider->bounds(1,1000);       // set min/max for slider
    minSlider->value(100);           // set initial value

    but = new Fl_Button( leftMargin, yOffsets[9], widgetWidth, widgetHeight, "PROCESS" );
    but->deactivate();

    progress = new Fl_Progress(leftMargin ,yOffsets[10], widgetWidth , widgetHeight);
    progress->minimum(0);                      // set progress range to be 0.0 ~ 1.0
    progress->maximum(1);
    progress->color(0x88888800);               // background color
    progress->selection_color(0x4444ff00);     // progress bar color
    progress->labelcolor(FL_WHITE);            // percent text color


/*
    progress->value(0);
    char percent[10];
    sprintf(percent, "%d%%", 0);
    progress->label(percent);              // update progress bar's label
    Fl::check();
    */
    //updateProgressBar(progress, 0.0f);



    win->end();


    directoryChooser->callback(open_cb , (void*)this  );

    but -> callback( processCallback , (void*)this);

    win->show();
    deactivateSliders();
    updateProgressBar( 0.0f);
}

void UI::deactivateSliders(){
    iterationsSlider->deactivate();
    timeStepSlider->deactivate();
    conductanceSlider->deactivate();
    thresholdSlider->deactivate();
    radiusSlider->deactivate();
    //distanceSlider->deactivate();
    minSlider->deactivate();
}

void UI::activateSliders(){
    iterationsSlider->activate();
    timeStepSlider->activate();
    conductanceSlider->activate();
    thresholdSlider->activate();
    radiusSlider->activate();
    //distanceSlider->activate();
    minSlider->activate();
}

void UI::open_cb(Fl_Widget *w, void *data) {
    UI* parent = ((UI*)data);
    // Create the file chooser, and show it
    Fl_File_Chooser chooser(".",                        // directory
                            "*",                        // filter
                            Fl_File_Chooser::DIRECTORY,     // chooser type
                            "Choose DICOM Series");        // title
    chooser.show();

    // Block until user picks something.
    //     (The other way to do this is to use a callback())
    //
    while(chooser.shown())
    { Fl::wait(); }

    // User hit cancel?
    if ( chooser.value() == NULL )
    { fprintf(stderr, "(User hit 'Cancel')\n"); return; }

    //update the input series widget
    parent->inputSeries->value(chooser.value());


    string directoryTemp(chooser.value());
    if(directoryTemp.back() != '/' ){
        directoryTemp+= '/';
    }

    parent->directoryValue = directoryTemp;
    parent->activateSliders();
    parent->but->activate();
    parent->clearOutputFile();
    parent->updateProgressBar(0.0f);



}

void UI::updateProgressBar( float percent){
    progress->value(percent);
    char percentLabel[40];
    sprintf(percentLabel, "%d%%", (int)(percent*100));
    progress->label(percentLabel);              // update progress bar's label
    Fl::check();
}




void UI::processCallback( Fl_Widget *w, void *data ) {
    UI* parent = ((UI*)data);
    parent->but->deactivate();
    parent->deactivateSliders();
    parent->directoryChooser->deactivate();
    Fl::check();

    parent->updateProgressBar( 0.0f);
    parent->updateOutputFile();

    parent->processFunction();

    parent->but->activate();
    parent->directoryChooser->activate();
    parent->activateSliders();
    Fl::check();
}

void UI::clearOutputFile(){
    outputFile->value("");
}


void UI::updateOutputFile(){
    char buffer [200];


    sprintf (buffer, "%soutput-%d-%.2f-%.2f-%d-%d-%d.txt", directoryValue.c_str() ,
             iterationsSlider->value(), timeStepSlider->value(), conductanceSlider->value(),
             thresholdSlider->value(), radiusSlider->value(),  minSlider->value());

    outputFile->value(buffer);
    //cout << buffer << endl;
}

void UI::setProcessFunction(callback_function pointer){

    processFunction = pointer;
}


string UI::getDirectory(){
    return directoryValue;
}

string UI::getOutputFile(){
    return outputFile->value();
}

int UI::getIterations(){
    return iterationsSlider->value();
}

float UI::getTimeStep(){
    return timeStepSlider->value();
}

float UI::getConductance(){
    return conductanceSlider->value();
}

int UI::getThreshold(){
    return thresholdSlider->value();
}

int UI::getRadius(){
    return radiusSlider->value();
}

/*
int UI::getDistance(){
    return distanceSlider->value();
}
 */

int UI::getMin(){
    return minSlider->value();
}
