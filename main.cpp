
#include "itkImage.h"
#include "CustomInteractor.h"
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


//For Edge Preserving Smoothing

#include "itkGradientAnisotropicDiffusionImageFilter.h"

//Threshold Segmentation
#include "itkThresholdSegmentationLevelSetImageFilter.h"
#include "itkOtsuThresholdImageFilter.h"
#include "itkThresholdImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"

#include <iostream>


using namespace std;

// Define viewport ranges
double xmins[4] = {0,.5,0,.5};
double xmaxs[4] = {0.5,1,0.5,1};
double ymins[4] = {0,0,.5,.5};
double ymaxs[4]= {0.5,0.5,1,1};


const unsigned int      Dimension = 3;
typedef float InputPixelType;
typedef float  OutputPixelType;
typedef itk::Image< InputPixelType, Dimension > InputImageType;
typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
typedef itk::ImageToVTKImageFilter < InputImageType > ConnectorType;

// helper class to format slice status message
class StatusMessage {
public:
    static std::string Format(int slice, int maxSlice) {
        std::stringstream tmp;
        tmp << "Slice Number  " << slice + 1 << "/" << maxSlice + 1;
        return tmp.str();
    }
};



int main(int argc, char **argv) {
    std::vector<vtkSmartPointer<vtkRenderWindowInteractor> > interactors;


    //typedef itk::Image< InputPixelType, Dimension > ImageType;
    typedef itk::ImageSeriesReader< InputImageType >        ReaderType;
    typedef itk::GDCMImageIO       ImageIOType;
    ImageIOType::Pointer dicomIO = ImageIOType::New();

    ReaderType::Pointer reader = ReaderType::New();

    reader->SetImageIO( dicomIO );
    typedef itk::GDCMSeriesFileNames NamesGeneratorType;
    NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();

    //nameGenerator->SetUseSeriesDetails( true );
    //nameGenerator->AddSeriesRestriction("0008|0021" );
    nameGenerator->SetDirectory( "/Users/mac/DICOMSLIDES");

/*
    typedef float InputPixelType;
    typedef float OutputPixelType;
    typedef itk::Image< InputPixelType, Dimension > InputImageType;
    typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
*/



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


    typedef itk::GradientAnisotropicDiffusionImageFilter<InputImageType,InputImageType > FilterType;
    FilterType::Pointer filter = FilterType::New();

    filter->SetInput( reader->GetOutput() );
    filter->SetNumberOfIterations( 1 );
    filter->SetTimeStep( 0.01 );
    filter->SetConductanceParameter( 3.0 );



    try
    {
        filter->Update();
    }
    catch( itk::ExceptionObject & error )
    {
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }


    /**
    *
    * SEGMENTATION
    *
    */



    typedef itk::BinaryThresholdImageFilter <InputImageType, InputImageType>
            BinaryThresholdImageFilterType;

    BinaryThresholdImageFilterType::Pointer binaryThresholdFilter
            = BinaryThresholdImageFilterType::New();
    binaryThresholdFilter->SetInput(reader->GetOutput());
    binaryThresholdFilter->SetLowerThreshold(500);
    //thresholdFilter->SetUpperThreshold(upperThreshold);
    //thresholdFilter->SetInsideValue(255);
    //thresholdFilter->SetOutsideValue(0);


    typedef itk::ThresholdImageFilter <InputImageType>
            ThresholdImageFilterType;

    ThresholdImageFilterType::Pointer thresholdFilter
            = ThresholdImageFilterType::New();
    thresholdFilter->SetInput(filter->GetOutput());
    //thresholdFilter->ThresholdOutside(lowerThreshold, upperThreshold);
    thresholdFilter->SetLower(400);
    thresholdFilter->SetOutsideValue(0);





    /**
     *
     * Connected Components
     *
     */

    //typedef itk::RGBPixel<unsigned char>         RGBPixelType;
    //typedef itk::Image<RGBPixelType, Dimension>  RGBImageType;
/*
    typedef unsigned char                       PixelType;
    typedef itk::RGBPixel<unsigned char>         RGBPixelType;
    typedef itk::Image<PixelType, Dimension>     ImageType;
    typedef itk::Image<RGBPixelType, Dimension>  RGBImageType;

    typedef itk::Image< unsigned short, Dimension > OutputImageType2;

    typedef itk::ConnectedComponentImageFilter <InputImageType, OutputImageType2 >
            ConnectedComponentImageFilterType;

    ConnectedComponentImageFilterType::Pointer connected =
            ConnectedComponentImageFilterType::New ();
    connected->SetInput(thresholdFilter->GetOutput() );
    connected->Update();

    std::cout << "Number of objects: " << connected->GetObjectCount() << std::endl;

    typedef itk::LabelToRGBImageFilter<OutputImageType2, RGBImageType> RGBFilterType;
    RGBFilterType::Pointer rgbFilter = RGBFilterType::New();
    rgbFilter->SetInput( connected->GetOutput() );

*/
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



    ConnectorType::Pointer binaryThresholdConnector = ConnectorType::New();
    binaryThresholdConnector->SetInput( binaryThresholdFilter->GetOutput() );
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
    volumeMapperBinary->SetInputData(binaryThresholdConnector->GetOutput());


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



      // Increase the size of the render window
     renWin->SetSize(1000, 1000);
    renWin->Render();

      // Interact with the data.
     //iren->Initialize();
     //iren->Start();
    interactors[0]->Start();





    return 0;
}


