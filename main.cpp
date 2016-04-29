
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

//For Edge Preserving Smoothing
#include "itkGradientAnisotropicDiffusionImageFilter.h"

//Threshold Segmentation
#include "itkThresholdSegmentationLevelSetImageFilter.h"

#include <iostream>


using namespace std;

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

    const unsigned int      Dimension = 3;
    typedef float InputPixelType;
    typedef float  OutputPixelType;
    typedef itk::Image< InputPixelType, Dimension > InputImageType;
    typedef itk::Image< OutputPixelType, Dimension > OutputImageType;

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

    typedef itk::ImageToVTKImageFilter < InputImageType > ConnectorType;
    ConnectorType::Pointer connector = ConnectorType::New();

    connector->SetInput( filter->GetOutput() );
    connector->Update();



    /**
     * VTK
     *
     */
    // Create the renderer, the render window, and the interactor. The renderer
    // draws into the render window, the interactor enables mouse- and
    // keyboard-based interaction with the scene.
    vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
    renWin->AddRenderer(ren);
    vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    iren->SetRenderWindow(renWin);
    renWin->SetSize(1000, 1000);


    vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> volumeMapper =
            vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
    volumeMapper->SetInputData(connector->GetOutput());


    vtkSmartPointer<vtkColorTransferFunction>volumeColor =
         vtkSmartPointer<vtkColorTransferFunction>::New();
      // volumeColor->AddRGBPoint(0,    0.0, 0.0, 0.0);
      // volumeColor->AddRGBPoint(500,  1.0, 0.5, 0.3);
      // volumeColor->AddRGBPoint(1000, 1.0, 0.5, 0.3);
      // volumeColor->AddRGBPoint(1150, 1.0, 1.0, 0.9);
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

       // Finally, add the volume to the renderer
       ren->AddViewProp(volume);

       // Set up an initial view of the volume.  The focal point will be the
       // center of the volume, and the camera position will be 400mm to the
       // patient's left (which is our right).
       vtkCamera *camera = ren->GetActiveCamera();
       double *c = volume->GetCenter();
       camera->SetFocalPoint(c[0], c[1], c[2]);
       camera->SetPosition(c[0] + 400, c[1], c[2]);
       camera->SetViewUp(0, 0, -4);

      // Increase the size of the render window
       renWin->SetSize(640, 480);

      // Interact with the data.
      iren->Initialize();
       iren->Start();

    return 0;
}