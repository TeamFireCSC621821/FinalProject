#include "itkScalarConnectedComponentImageFilter.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageToVTKImageFilter.h"
#include "vtkImageViewer.h"
#include "vtkRenderWindowInteractor.h"
#include "itkImageSeriesReader.h"
#include "itkCurvatureFlowImageFilter.h"
#include <vtkDICOMImageReader.h>
#include "itkGDCMSeriesFileNames.h"
#include "itkGDCMImageIO.h"
#include <vtkImageSliceMapper.h>
#include <vtkImageMapper.h>
#include <vtkImageSlice.h>
#include <vtkVolumeProperty.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageViewer2.h>
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>
#include <vtkImageShiftScale.h>
#include <vtkImageClip.h>
#include <vtkVolumeRayCastMapper.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkCamera.h>
#include "itkBinaryThresholdImageFilter.h"
#include "itkFastMarchingImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkLabelToRGBImageFilter.h"
#include <vtkObjectFactory.h>
#include <vtkInteractorStyleImage.h>
#include <vtkActor2D.h>
#include "itkBinaryMorphologicalOpeningImageFilter.h"
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>
#include "itkBinaryBallStructuringElement.h"
#include "itkSubtractImageFilter.h"
#include <FL/Fl.H>
#include <FL/Fl_window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Progress.H>
#include <math.h>
#include <sstream>
#include <string>
#include "UI.h"


//For Edge Preserving Smoothing

#include "itkGradientAnisotropicDiffusionImageFilter.h"

//Threshold Segmentation
#include "itkThresholdSegmentationLevelSetImageFilter.h"
#include "itkOtsuThresholdImageFilter.h"
#include "itkThresholdImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"

#include <iostream>
#include <fstream>

using namespace std;

// Define viewport ranges
double xmins[4] = {0,.5,0,.5};
double xmaxs[4] = {0.5,1,0.5,1};
double ymins[4] = {0,0,.5,.5};
double ymaxs[4]= {0.5,0.5,1,1};

// Define viewport ranges
double xmins2[3] = {0,0.33 , 0.66};
double xmaxs2[3] = {0.33,0.66,1};
double ymins2[3] = {0,0,0};
double ymaxs2[3]= {1,1,1};



const unsigned int      Dimension = 3;
typedef float InputPixelType;
typedef float  OutputPixelType;
typedef itk::Image< InputPixelType, Dimension > InputImageType;
typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
typedef itk::ImageToVTKImageFilter < InputImageType > ConnectorType;

void  processCallback( Fl_Widget*, void* );
void process();
void updateProgressBar( Fl_Progress *progress , float percent);
void open_cb(Fl_Widget*, void*);
void updateOutputFile();
void deactivateSliders();


/**
 * Declare global widget pointers so they can talk to eachother without having to have a bunch of
 * pointers for arguments
 */



UI* ui;


// helper class to format slice status message
class StatusMessage {
public:
    static std::string Format(int slice, int maxSlice) {
        std::stringstream tmp;
        tmp << "Slice Number  " << slice + 1 << "/" << maxSlice + 1;
        return tmp.str();
    }
};

class CustomInteractor : public vtkInteractorStyleImage
{
public:
    static CustomInteractor* New();
    vtkTypeMacro(CustomInteractor, vtkInteractorStyleImage);

protected:
    //vtkImageViewer2* _ImageViewer;
    vtkTextMapper* _StatusMapper;
    vtkImageMapper* _ImageMapperLeft;
    vtkImageMapper* _ImageMapperRight;
    vtkRenderer* _ImageRendererLeft;
    vtkRenderer* _ImageRendererRight;
    vtkImageMapper* _ImageMapperRight2;
    vtkRenderer* _ImageRendererRight2;
    vtkRenderWindow* _RenderWindow;

    int _Slice;
    int _MinSlice;
    int _MaxSlice;

public:
    void SetImageViewer(vtkImageViewer2* imageViewer) {
        //_ImageViewer = imageViewer;
        //_MinSlice = imageViewer->GetSliceMin();
        //_MaxSlice = imageViewer->GetSliceMax();
        _Slice = _MinSlice;
        //cout << "Slicer: Min = " << _MinSlice << ", Max = " << _MaxSlice << std::endl;
    }

    void SetStatusMapper(vtkTextMapper* statusMapper) {
        _StatusMapper = statusMapper;

    }

    int getMinSlice(){
        return _MinSlice;
    }

    int getMaxSlice(){
        return _MaxSlice;
    }


    void SetMapper1(vtkImageMapper* imageMapper){
        _ImageMapperLeft = imageMapper;
        _MinSlice = imageMapper->GetWholeZMin();
        _MaxSlice = imageMapper->GetWholeZMax();
        _Slice = _MinSlice;
    }

    void SetMapper2(vtkImageMapper* imageMapper){
        _ImageMapperRight = imageMapper;
    }

    void SetMapper3(vtkImageMapper* imageMapper){
        _ImageMapperRight2 = imageMapper;
    }

    void SetRenderer1(vtkRenderer* imageRenderer){
        _ImageRendererLeft = imageRenderer;
    }

    void SetRenderer3(vtkRenderer* imageRenderer){
        _ImageRendererRight2 = imageRenderer;
    }

    void SetRenderer2(vtkRenderer* imageRenderer){
        _ImageRendererRight = imageRenderer;
    }

    void SetRenderWindow(vtkRenderWindow* window){
        _RenderWindow = window;
    }


protected:
    void MoveSliceForward() {
        if(_Slice < _MaxSlice) {
            _Slice += 1;
            cout << "MoveSliceForward::Slice = " << _Slice << std::endl;
            //_ImageViewer->SetSlice(_Slice);
            _ImageMapperRight->SetZSlice(_Slice);
            _ImageMapperLeft->SetZSlice(_Slice);
            _ImageRendererLeft->Render();
            _ImageRendererRight->Render();

            _ImageMapperRight2->SetZSlice(_Slice);
            _ImageRendererRight2->Render();

            std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
            _StatusMapper->SetInput(msg.c_str());
            _RenderWindow->Render();
            //_ImageViewer->Render();
            //_ImageMapperLeft->Render();
            //_ImageMapperRight->Render();

        }
    }

    void MoveSliceBackward() {
        if(_Slice > _MinSlice) {
            _Slice -= 1;
            cout << "MoveSliceBackward::Slice = " << _Slice << std::endl;
            //_ImageViewer->SetSlice(_Slice);
            _ImageMapperLeft->SetZSlice(_Slice);
            _ImageMapperRight->SetZSlice(_Slice);
            _ImageMapperRight2->SetZSlice(_Slice);
            _ImageRendererRight2->Render();


            std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
            _StatusMapper->SetInput(msg.c_str());
            _RenderWindow->Render();
            //_ImageViewer->Render();
        }
    }


    virtual void OnKeyDown() {
        std::string key = this->GetInteractor()->GetKeySym();
        if(key.compare("Up") == 0) {
            //cout << "Up arrow key was pressed." << endl;
            MoveSliceForward();
        }
        else if(key.compare("Down") == 0) {
            //cout << "Down arrow key was pressed." << endl;
            MoveSliceBackward();
        }
        // forward event
        vtkInteractorStyleImage::OnKeyDown();
    }


    virtual void OnMouseWheelForward() {
        //std::cout << "Scrolled mouse wheel forward." << std::endl;
        MoveSliceForward();
        // don't forward events, otherwise the image will be zoomed
        // in case another interactorstyle is used (e.g. trackballstyle, ...)
        // vtkInteractorStyleImage::OnMouseWheelForward();
    }


    virtual void OnMouseWheelBackward() {
        //std::cout << "Scrolled mouse wheel backward." << std::endl;
        if(_Slice > _MinSlice) {
            MoveSliceBackward();
        }
        // don't forward events, otherwise the image will be zoomed
        // in case another interactorstyle is used (e.g. trackballstyle, ...)
        // vtkInteractorStyleImage::OnMouseWheelBackward();
    }
};









vtkStandardNewMacro( CustomInteractor);



int main(){

    ui  = new UI();
    ui->setProcessFunction(&process);
    return  Fl::run();

}






void process() {

    std::vector<vtkSmartPointer<vtkRenderWindowInteractor> > interactors;


    //typedef itk::Image< InputPixelType, Dimension > ImageType;
    typedef itk::ImageSeriesReader< InputImageType >        ReaderType;
    typedef itk::GDCMImageIO       ImageIOType;
    ImageIOType::Pointer dicomIO = ImageIOType::New();

    ReaderType::Pointer reader = ReaderType::New();

    reader->SetImageIO( dicomIO );
    typedef itk::GDCMSeriesFileNames NamesGeneratorType;
    NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();


    nameGenerator->SetDirectory(ui->getDirectory());




    typedef std::vector< std::string >    SeriesIdContainer;
    const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();
    std::cout << seriesUID.size() << std::endl;
    SeriesIdContainer::const_iterator seriesItr = seriesUID.begin();
    SeriesIdContainer::const_iterator seriesEnd = seriesUID.end();
    while( seriesItr != seriesEnd )
    {
        std::cout << seriesItr->c_str() << std::endl;
        seriesItr++;
    }
    std::string seriesIdentifier;
    seriesIdentifier = seriesUID.begin()->c_str();
    std::cout << seriesIdentifier.c_str() << std::endl;

    typedef std::vector< std::string >   FileNamesContainer;
    FileNamesContainer fileNames;
    fileNames = nameGenerator->GetFileNames( seriesIdentifier );

    reader->SetFileNames( fileNames );
    try
    {
        reader->Update();
    }
    catch (itk::ExceptionObject &ex)
    {
        std::cout << ex << std::endl;
    }

    ui->updateProgressBar(0.1f);




    /**
     *
     * Smoothing Filter
     *
     */

    typedef itk::GradientAnisotropicDiffusionImageFilter<InputImageType,InputImageType > FilterType;
    FilterType::Pointer filter = FilterType::New();

    filter->SetInput( reader->GetOutput() );
    filter->SetNumberOfIterations( ui->getIterations() );
    filter->SetTimeStep( ui->getTimeStep() );
    filter->SetConductanceParameter( ui->getConductance() );



    try
    {
        filter->Update();
    }
    catch( itk::ExceptionObject & error )
    {
        std::cerr << "Error: " << error << std::endl;
        //return EXIT_FAILURE;
    }

    ui->updateProgressBar(0.2f);


    /**
    *
    * THRESHOLDING
    *
    */



    typedef itk::BinaryThresholdImageFilter <InputImageType, InputImageType>
            BinaryThresholdImageFilterType;

    BinaryThresholdImageFilterType::Pointer binaryThresholdFilter
            = BinaryThresholdImageFilterType::New();
    binaryThresholdFilter->SetInput(filter->GetOutput());
    binaryThresholdFilter->SetLowerThreshold(ui->getThreshold());
    binaryThresholdFilter->Update();

    ui->updateProgressBar(0.3f);

    typedef itk::ThresholdImageFilter <InputImageType>
            ThresholdImageFilterType;

    ThresholdImageFilterType::Pointer thresholdFilter
            = ThresholdImageFilterType::New();
    thresholdFilter->SetInput(filter->GetOutput());
    //thresholdFilter->ThresholdOutside(lowerThreshold, upperThreshold);
    thresholdFilter->SetLower(ui->getThreshold());
    thresholdFilter->SetOutsideValue(0);
    thresholdFilter->Update();

    ui->updateProgressBar(0.4f);

    /**
    *
    * Morphological Operations
    *
    */

    typedef itk::BinaryBallStructuringElement<InputImageType::PixelType, InputImageType::ImageDimension>
            StructuringElementType;
    StructuringElementType structuringElement;
    structuringElement.SetRadius(ui->getRadius());
    structuringElement.CreateStructuringElement();

    typedef itk::BinaryMorphologicalClosingImageFilter <InputImageType, InputImageType, StructuringElementType>
            BinaryMorphologicalClosingImageFilterType;
    BinaryMorphologicalClosingImageFilterType::Pointer closingFilter
            = BinaryMorphologicalClosingImageFilterType::New();
    closingFilter->SetInput(binaryThresholdFilter->GetOutput());
    closingFilter->SetKernel(structuringElement);
    closingFilter->Update();


    ui->updateProgressBar(0.5f);

    /**
     *
     * Connected Components
     *
     */



    typedef unsigned int                           LabelPixelType;
    typedef itk::Image<LabelPixelType, Dimension > LabelImageType;

    typedef itk::ScalarConnectedComponentImageFilter <InputImageType, LabelImageType >
            ConnectedComponentImageFilterType;


    ConnectedComponentImageFilterType::Pointer connected =
            ConnectedComponentImageFilterType::New ();
    connected->SetInput(closingFilter->GetOutput());
    connected->SetDistanceThreshold(ui->getDistance());

    ui->updateProgressBar(0.6f);

    typedef itk::RelabelComponentImageFilter <LabelImageType, LabelImageType >
            RelabelFilterType;
    RelabelFilterType::Pointer relabel =
            RelabelFilterType::New();
    RelabelFilterType::ObjectSizeType minSize = ui->getMin();

    relabel->SetInput(connected->GetOutput());
    relabel->SetMinimumObjectSize(minSize);
    relabel->Update();

    int labelCount = relabel->GetNumberOfObjects();
    std::cout << "Number of labels: "
    << labelCount << endl;

    ofstream out(ui->getOutputFile());
    for(int n = 0; n < labelCount; n++){
        out  << n << "," << relabel->GetSizeOfObjectsInPixels()[n] << "," <<
                relabel->GetSizeOfObjectsInPhysicalUnits()[n] << endl;
    }
    out.close();

    ui->updateProgressBar(0.7f);

    /**
    *
    * Color Connected Components
    *
    */

    typedef itk::RGBPixel<unsigned char>           RGBPixelType;
    typedef itk::Image<RGBPixelType, Dimension>    RGBImageType;
    RGBPixelType pixel;
    pixel.SetRed(255);
    pixel.SetBlue(255);
    pixel.SetGreen(255);

    ui->updateProgressBar(0.8f);

    typedef itk::LabelToRGBImageFilter<LabelImageType, RGBImageType> RGBFilterType;
    RGBFilterType::Pointer rgbFilter =
            RGBFilterType::New();
    rgbFilter->SetInput( relabel->GetOutput() );
    rgbFilter->SetBackgroundColor(pixel);
    rgbFilter->Update();

    ui->updateProgressBar(0.9f);


    /**
    *
    * VTK
    *
    */

    //typedef itk::ImageToVTKImageFilter < RGBImageType > ConnectorType;
    ConnectorType::Pointer rawConnector = ConnectorType::New();
    //connector->SetInput( rgbFilter->GetOutput() );
    rawConnector->SetInput( reader->GetOutput() );
    //connector->SetInput( filter->GetOutput() );
    //connector->SetInput( reader->GetOutput() );
    rawConnector->Update();

    ConnectorType::Pointer preConnector = ConnectorType::New();
    preConnector->SetInput( binaryThresholdFilter->GetOutput() );
    preConnector->Update();

    ConnectorType::Pointer closingConnector = ConnectorType::New();
    closingConnector->SetInput( closingFilter->GetOutput() );
    closingConnector->Update();




    typedef itk::ImageToVTKImageFilter < LabelImageType > ConnectorType2;
    ConnectorType2::Pointer binaryThresholdConnector = ConnectorType2::New();
    binaryThresholdConnector->SetInput( relabel->GetOutput() );
    binaryThresholdConnector->Update();




    //typedef itk::ImageToVTKImageFilter < RGBImageType > ConnectorType;
    ConnectorType::Pointer connector = ConnectorType::New();
    //connector->SetInput( rgbFilter->GetOutput() );
    connector->SetInput( thresholdFilter->GetOutput() );
    //connector->SetInput( filter->GetOutput() );
    //connector->SetInput( reader->GetOutput() );
    connector->Update();

    vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();

    interactors.push_back(iren);
    iren->SetRenderWindow(renWin);



    // Create the renderer, the render window, and the interactor. The renderer
    // draws into the render window, the interactor enables mouse- and
    // keyboard-based interaction with the scene.

    vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();


    vtkSmartPointer<vtkRenderer> rendererVolOriginal = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderer> rendererBinaryOutput = vtkSmartPointer<vtkRenderer>::New();



    renWin->AddRenderer(rendererVolOriginal);
    rendererVolOriginal->SetViewport(xmins[2],ymins[2],xmaxs[2],ymaxs[2]);

    renWin->AddRenderer(ren);
    ren->SetViewport(xmins[3],ymins[3],xmaxs[3],ymaxs[3]);

    renWin->AddRenderer(rendererBinaryOutput);
    rendererBinaryOutput->SetViewport(xmins[1],ymins[1],xmaxs[1],ymaxs[1]);





    vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> volumeMapper =
            vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
    volumeMapper->SetInputData(connector->GetOutput());

    vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> volumeMapperOriginal =
            vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
    volumeMapperOriginal->SetInputData(rawConnector->GetOutput());

    vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> volumeMapperBinary =
            vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
    volumeMapperBinary->SetInputData(closingConnector->GetOutput());


    vtkSmartPointer<vtkColorTransferFunction>volumeColor =
         vtkSmartPointer<vtkColorTransferFunction>::New();
       //volumeColor->AddRGBPoint(0,    0.0, 0.0, 0.0);
       //volumeColor->AddRGBPoint(500,  1.0, 0.5, 0.3);
      // volumeColor->AddRGBPoint(1000, 1.0, 0.5, 0.3);
    //   volumeColor->AddRGBPoint(1150, 1.0, 1.0, 0.9);
    volumeColor->AddRGBPoint(1150,    0.0, 0.0, 0.0);
    volumeColor->AddRGBPoint(1000,  1.0, 0.5, 0.3);
    volumeColor->AddRGBPoint(500, 1.0, 0.5, 0.3);
    volumeColor->AddRGBPoint(0, 1.0, 1.0, 0.9);


    // The opacity transfer function is used to control the opacity
       // of different tissue types.
    vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity =
         vtkSmartPointer<vtkPiecewiseFunction>::New();
    volumeScalarOpacity->AddPoint(0,    0.00);
    volumeScalarOpacity->AddPoint(500,  0.15);
    volumeScalarOpacity->AddPoint(1000, 0.15);
    volumeScalarOpacity->AddPoint(1150, 0.85);

    //volumeScalarOpacity->AddPoint(1150,    0.00);
    //volumeScalarOpacity->AddPoint(1000,  0.15);
    //volumeScalarOpacity->AddPoint(500, 0.15);
    //volumeScalarOpacity->AddPoint(0, 0.85);

       // The gradient opacity function is used to decrease the opacity
       // in the "flat" regions of the volume while maintaining the opacity
       // at the boundaries between tissue types.  The gradient is measured
       // as the amount by which the intensity changes over unit distance.
       // For most medical data, the unit distance is 1mm.
       vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity =
         vtkSmartPointer<vtkPiecewiseFunction>::New();
       volumeGradientOpacity->AddPoint(0,   0.0);
       volumeGradientOpacity->AddPoint(90,  0.5);
       volumeGradientOpacity->AddPoint(100, 1.0);


       vtkSmartPointer<vtkVolumeProperty> volumeProperty =
         vtkSmartPointer<vtkVolumeProperty>::New();
       volumeProperty->SetColor(volumeColor);
       volumeProperty->SetScalarOpacity(volumeScalarOpacity);
       volumeProperty->SetGradientOpacity(volumeGradientOpacity);
       volumeProperty->SetInterpolationTypeToLinear();
       volumeProperty->ShadeOn();
       volumeProperty->SetAmbient(0.4);
       volumeProperty->SetDiffuse(0.6);
       volumeProperty->SetSpecular(0.2);
       // The vtkVolume is a vtkProp3D (like a vtkActor) and controls the position
       // and orientation of the volume in world coordinates.
       vtkSmartPointer<vtkVolume> volume =
        vtkSmartPointer<vtkVolume>::New();
       volume->SetMapper(volumeMapper);
       volume->SetProperty(volumeProperty);


    vtkSmartPointer<vtkVolume> volumeOriginal =
            vtkSmartPointer<vtkVolume>::New();
    volumeOriginal->SetMapper(volumeMapperOriginal);
    volumeOriginal->SetProperty(volumeProperty);


    vtkSmartPointer<vtkVolume> volumeBinary =
            vtkSmartPointer<vtkVolume>::New();
    volumeBinary->SetMapper(volumeMapperBinary);
    volumeBinary->SetProperty(volumeProperty);

       // Finally, add the volume to the renderer
     ren->SetBackground(1,1,1);
     ren->AddViewProp(volume);
     ren->ResetCamera();



    rendererVolOriginal ->SetBackground(1,1,1);
    rendererVolOriginal ->AddViewProp(volumeOriginal);
    rendererVolOriginal ->ResetCamera();

    rendererBinaryOutput ->SetBackground(1,1,1);
    rendererBinaryOutput ->AddViewProp(volumeBinary);
    rendererBinaryOutput ->ResetCamera();


    rendererVolOriginal->SetActiveCamera(rendererBinaryOutput->GetActiveCamera());
    ren->SetActiveCamera(rendererVolOriginal->GetActiveCamera());




      // Increase the size of the render window
     renWin->SetSize(600, 600);
    renWin->Render();

      // Interact with the data.
     //iren->Initialize();
     //iren->Start();

    /**
    *
    * Visualize original
    *
    */


    ConnectorType::Pointer originalSliceConnector = ConnectorType::New();
    originalSliceConnector->SetInput( reader->GetOutput() );
    originalSliceConnector->Update();

    int *dimensions = connector->GetOutput()->GetDimensions();




    vtkSmartPointer<vtkRenderWindow> renderWindow =
            vtkSmartPointer<vtkRenderWindow>::New();

    renderWindow->SetSize(dimensions[0] * 3, dimensions[1]);


    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
            vtkSmartPointer<vtkRenderWindowInteractor>::New();

    vtkSmartPointer<vtkRenderer> rendererLeft =
            vtkSmartPointer<vtkRenderer>::New();

    vtkSmartPointer<vtkRenderer> rendererRight =
            vtkSmartPointer<vtkRenderer>::New();

    vtkSmartPointer<vtkRenderer> rendererRight2 =
            vtkSmartPointer<vtkRenderer>::New();

    renderWindow->AddRenderer(rendererLeft);
    renderWindow->AddRenderer(rendererRight);
    renderWindow->AddRenderer(rendererRight2);
    rendererLeft->SetViewport(xmins2[0],ymins2[0],xmaxs2[0],ymaxs2[0]);
    rendererRight->SetViewport(xmins2[1],ymins2[1],xmaxs2[1],ymaxs2[1]);
    rendererRight2->SetViewport(xmins2[2],ymins2[2],xmaxs2[2],ymaxs2[2]);

    interactors.push_back(renderWindowInteractor);

    vtkSmartPointer<CustomInteractor> customInteractorStyle =
            vtkSmartPointer<CustomInteractor>::New();


    renderWindowInteractor->SetRenderWindow(renderWindow);
    renderWindow->Render();
    renderWindowInteractor->SetInteractorStyle(customInteractorStyle );

    vtkSmartPointer<vtkImageMapper> imageMapperLeft = vtkSmartPointer<vtkImageMapper>::New();
    imageMapperLeft->SetInputData(rawConnector->GetOutput());


    typedef itk::ImageToVTKImageFilter < RGBImageType > ColoredConnectorType;
    ColoredConnectorType::Pointer coloredConnector = ColoredConnectorType::New();
    coloredConnector->SetInput( rgbFilter->GetOutput() );
    coloredConnector->Update();



    vtkSmartPointer<vtkImageMapper> imageMapperRight2 = vtkSmartPointer<vtkImageMapper>::New();
    //imageMapperRight2->SetInputData(binaryThresholdConnector->GetOutput());
    //imageMapperRight2->SetInputData(closingConnector->GetOutput());
    imageMapperRight2->SetInputData(coloredConnector->GetOutput());


    vtkSmartPointer<vtkImageMapper> imageMapperRight = vtkSmartPointer<vtkImageMapper>::New();
    imageMapperRight->SetInputData(preConnector->GetOutput());


    customInteractorStyle->SetMapper1(imageMapperLeft);
    customInteractorStyle->SetMapper2(imageMapperRight);
    customInteractorStyle->SetMapper3(imageMapperRight2);
    customInteractorStyle->SetRenderer1(rendererLeft);
    customInteractorStyle->SetRenderer2(rendererRight);
    customInteractorStyle->SetRenderer3(rendererRight2);
    customInteractorStyle->SetRenderWindow(renderWindow);

    vtkSmartPointer<vtkTextProperty> sliceTextProp = vtkSmartPointer<vtkTextProperty>::New();
    sliceTextProp->SetFontFamilyToCourier();
    sliceTextProp->SetFontSize(30);
    sliceTextProp->SetVerticalJustificationToBottom();
    sliceTextProp->SetJustificationToLeft();

    vtkSmartPointer<vtkTextMapper> sliceTextMapper = vtkSmartPointer<vtkTextMapper>::New();
    std::string msg = StatusMessage::Format(customInteractorStyle->getMinSlice(), customInteractorStyle->getMaxSlice());
    sliceTextMapper->SetInput(msg.c_str());
    sliceTextMapper->SetTextProperty(sliceTextProp);

    vtkSmartPointer<vtkActor2D> sliceTextActor = vtkSmartPointer<vtkActor2D>::New();
    sliceTextActor->SetMapper(sliceTextMapper);
    sliceTextActor->SetPosition(15, 10);

    // usage hint message
    vtkSmartPointer<vtkTextProperty> usageTextProp = vtkSmartPointer<vtkTextProperty>::New();
    usageTextProp->SetFontFamilyToCourier();
    usageTextProp->SetFontSize(25);
    usageTextProp->SetVerticalJustificationToTop();
    usageTextProp->SetJustificationToLeft();

    vtkSmartPointer<vtkTextMapper> usageTextMapper = vtkSmartPointer<vtkTextMapper>::New();
    usageTextMapper->SetInput("Original Image Set");
    usageTextMapper->SetTextProperty(usageTextProp);

    vtkSmartPointer<vtkActor2D> usageTextActor = vtkSmartPointer<vtkActor2D>::New();
    usageTextActor->SetMapper(usageTextMapper);
    usageTextActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedDisplay();
    usageTextActor->GetPositionCoordinate()->SetValue( 0.05, 0.95);

    vtkSmartPointer<vtkTextMapper> usageTextMapperMid = vtkSmartPointer<vtkTextMapper>::New();
    usageTextMapperMid->SetInput("Post Threshold Filter");
    usageTextMapperMid->SetTextProperty(usageTextProp);

    vtkSmartPointer<vtkActor2D> usageTextActorMid = vtkSmartPointer<vtkActor2D>::New();
    usageTextActorMid->SetMapper(usageTextMapperMid);
    usageTextActorMid->GetPositionCoordinate()->SetCoordinateSystemToNormalizedDisplay();
    usageTextActorMid->GetPositionCoordinate()->SetValue( 0.35, 0.95);

    vtkSmartPointer<vtkTextMapper> usageTextMapperRight = vtkSmartPointer<vtkTextMapper>::New();
    usageTextMapperRight->SetInput("Post Component Separation");
    usageTextMapperRight->SetTextProperty(usageTextProp);

    vtkSmartPointer<vtkActor2D> usageTextActorRight = vtkSmartPointer<vtkActor2D>::New();
    usageTextActorRight->SetMapper(usageTextMapperRight);
    usageTextActorRight->GetPositionCoordinate()->SetCoordinateSystemToNormalizedDisplay();
    usageTextActorRight->GetPositionCoordinate()->SetValue( 0.75, 0.95);


    customInteractorStyle->SetStatusMapper(sliceTextMapper);

    rendererRight->GetActiveCamera()->ParallelProjectionOn();
    rendererRight2->GetActiveCamera()->ParallelProjectionOn();
    rendererLeft->GetActiveCamera()->ParallelProjectionOn();

    vtkSmartPointer<vtkActor2D> imageActorLeft = vtkSmartPointer<vtkActor2D>::New();
    imageActorLeft ->SetMapper(imageMapperLeft);
    rendererLeft->AddActor2D(imageActorLeft);
    rendererLeft->SetBackground(1,1,1);

    vtkSmartPointer<vtkActor2D> imageActorRight = vtkSmartPointer<vtkActor2D>::New();
    imageActorRight ->SetMapper(imageMapperRight);
    rendererRight->AddActor2D(imageActorRight);

    vtkSmartPointer<vtkActor2D> imageActorRight2 = vtkSmartPointer<vtkActor2D>::New();
    imageActorRight2 ->SetMapper(imageMapperRight2);
    rendererRight2->AddActor2D(imageActorRight2);

    rendererLeft->AddActor2D(sliceTextActor);
    rendererLeft->AddActor2D(usageTextActor);

    rendererLeft->Render();
    rendererLeft->ResetCamera();
    rendererLeft->Render();

    rendererRight->AddActor2D(usageTextActorMid);
    rendererRight->Render();
    rendererRight->ResetCamera();
    rendererRight->Render();


    rendererRight2->AddActor2D(usageTextActorRight);
    rendererRight2->Render();
    rendererRight2->ResetCamera();
    rendererRight2->Render();


    ui->updateProgressBar(1.0f);
    interactors[1]->Start();





    //return 0;
}


