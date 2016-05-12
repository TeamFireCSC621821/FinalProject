
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
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>
#include "itkCheckerBoardImageFilter.h"
#include "itkImageIterator.h"

//Registration Imports
#include "itkDemonsRegistrationFilter.h"
#include "itkHistogramMatchingImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkWarpImageFilter.h"

//For Edge Preserving Smoothing

#include "itkGradientAnisotropicDiffusionImageFilter.h"

//Threshold Segmentation
#include "itkThresholdSegmentationLevelSetImageFilter.h"
#include "itkOtsuThresholdImageFilter.h"
#include "itkThresholdImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"

#include <iostream>
#include <itkImageSeriesWriter.h>

#include <string.h>
#include <sstream>

using namespace std;

// Define viewport ranges
double xmins[4] = {0,.5,0,.5};
double xmaxs[4] = {0.5,1,0.5,1};
double ymins[4] = {0,0,.5,.5};
double ymaxs[4]= {0.5,0.5,1,1};

// Define viewport ranges
double xmins2[2] = {0,.5};
double xmaxs2[2] = {0.5,1};
double ymins2[2] = {0,0};
double ymaxs2[2]= {1,1};


const unsigned int      Dimension = 3;
typedef float InputPixelType;
typedef float  OutputPixelType;
typedef itk::Image< InputPixelType, Dimension > InputImageType;
typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
typedef itk::ImageToVTKImageFilter < InputImageType > ConnectorType;

//registration
//const unsigned int Dimension = 3;
typedef float PixelType;
typedef itk::Image< PixelType, Dimension > FixedImageType;
typedef itk::Image< PixelType, Dimension > MovingImageType;

typedef itk::ImageSeriesReader< FixedImageType >        FixedImageReaderType;
typedef itk::ImageSeriesReader< MovingImageType >        MovingImageReaderType;

typedef itk::GradientAnisotropicDiffusionImageFilter<FixedImageType,FixedImageType > SmoothingFilterType;

//checkerboard typedefs
typedef itk::CheckerBoardImageFilter< FixedImageType > CheckerBoardFilterType;


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


    void SetMapper1(vtkImageMapper* imageMapper){
        _ImageMapperLeft = imageMapper;
        _MinSlice = imageMapper->GetWholeZMin();
        _MaxSlice = imageMapper->GetWholeZMax();
        _Slice = _MinSlice;
    }

    void SetMapper2(vtkImageMapper* imageMapper){
        _ImageMapperRight = imageMapper;
    }

    void SetRenderer1(vtkRenderer* imageRenderer){
        _ImageRendererLeft = imageRenderer;
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
            _RenderWindow->Render();
            std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
            //_StatusMapper->SetInput(msg.c_str());
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
            _RenderWindow->Render();
            //std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
            //_StatusMapper->SetInput(msg.c_str());
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
SmoothingFilterType::Pointer Smoothing(FixedImageReaderType::Pointer);
//SmoothingFilterType::Pointer Smoothing(MovingImageReaderType::Pointer);

int main(int argc, char **argv) {
    std::vector<vtkSmartPointer<vtkRenderWindowInteractor> > interactors;



    //This is for fixedImage loading
    //typedef itk::Image< InputPixelType, Dimension > ImageType;

    typedef itk::GDCMImageIO       ImageIOType;
    ImageIOType::Pointer fixedDicomIO = ImageIOType::New();
    ImageIOType::Pointer movingDicomIO = ImageIOType::New();

    FixedImageReaderType::Pointer fixedImageReader = FixedImageReaderType::New();
    MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();

    fixedImageReader->SetImageIO(fixedDicomIO);
    movingImageReader->SetImageIO(movingDicomIO);
    //reader->SetImageIO( dicomIO );
    typedef itk::GDCMSeriesFileNames NamesGeneratorType;
    NamesGeneratorType::Pointer fixedNameGenerator = NamesGeneratorType::New();
    NamesGeneratorType::Pointer movingNameGenerator = NamesGeneratorType::New();

    //nameGenerator->SetUseSeriesDetails( true );
    //nameGenerator->AddSeriesRestriction("0008|0021" );
	fixedNameGenerator->SetDirectory("C:/Fixed");
    movingNameGenerator->SetDirectory("C:/Moving");


    typedef std::vector< std::string >    SeriesIdContainer;
    const SeriesIdContainer & fixedSeriesUID = fixedNameGenerator->GetSeriesUIDs();
    std::cout << fixedSeriesUID.size() << std::endl;
    SeriesIdContainer::const_iterator fixedSeriesItr = fixedSeriesUID.begin();
    SeriesIdContainer::const_iterator fixedSeriesEnd = fixedSeriesUID.end();
    while( fixedSeriesItr != fixedSeriesEnd )
    {
        std::cout << fixedSeriesItr->c_str() << std::endl;
        fixedSeriesItr++;
    }
    std::string fixedSeriesIdentifier;
    fixedSeriesIdentifier = fixedSeriesUID.begin()->c_str();
    std::cout << fixedSeriesIdentifier.c_str() << std::endl;

    typedef std::vector< std::string >   FileNamesContainer;
    FileNamesContainer fixedFileNames;
    fixedFileNames = fixedNameGenerator->GetFileNames( fixedSeriesIdentifier );

    fixedImageReader->SetFileNames( fixedFileNames );
    try
    {
        fixedImageReader->Update();
    }
    catch (itk::ExceptionObject &ex)
    {
        std::cout << ex << std::endl;
    }




    //moving
    //typedef std::vector< std::string >    SeriesIdContainer;
    const SeriesIdContainer & movingSeriesUID = movingNameGenerator->GetSeriesUIDs();
    std::cout << movingSeriesUID.size() << std::endl;
    SeriesIdContainer::const_iterator movingSeriesItr = movingSeriesUID.begin();
    SeriesIdContainer::const_iterator movingSeriesEnd = movingSeriesUID.end();
    while( movingSeriesItr != movingSeriesEnd )
    {
        std::cout << movingSeriesItr->c_str() << std::endl;
        movingSeriesItr++;
    }
    std::string movingSeriesIdentifier;
    movingSeriesIdentifier = movingSeriesUID.begin()->c_str();
    std::cout << movingSeriesIdentifier.c_str() << std::endl;

    FileNamesContainer movingFileNames;
    movingFileNames = movingNameGenerator->GetFileNames( movingSeriesIdentifier );

    movingImageReader->SetFileNames( movingFileNames );
    try
    {
        movingImageReader->Update();
    }
    catch (itk::ExceptionObject &ex)
    {
        std::cout << ex << std::endl;
    }

    SmoothingFilterType::Pointer fixedFilter = Smoothing(fixedImageReader);
    SmoothingFilterType::Pointer movingFilter = Smoothing(movingImageReader);

    /*
     * Thirion's Demon Registration
     *
     */
    int histogramLevel = 512;
    int histogramMatchPoints = 7;
    int numberOfIterations = 10;
    float standardDeviations = 1.0;

    typedef float InternalPixelType;
    typedef itk::Image<InternalPixelType, Dimension> InternalImageType;
    typedef itk::CastImageFilter<FixedImageType, InternalImageType> FixedImageCasterType;
    typedef itk::CastImageFilter<MovingImageType, InternalImageType> MovingImageCasterType;

    FixedImageCasterType::Pointer fixedImageCaster = FixedImageCasterType::New();
    MovingImageCasterType::Pointer movingImageCaster = MovingImageCasterType::New();

    fixedImageCaster->SetInput(fixedFilter->GetOutput());
    movingImageCaster->SetInput(movingFilter->GetOutput());

	
    for(histogramMatchPoints = 6; histogramMatchPoints <= 8; histogramMatchPoints++) {
        for(standardDeviations = 0.5; standardDeviations <= 1.5; standardDeviations+=0.5) {

			cout << "Starting Histogram Matching";
			
            typedef itk::HistogramMatchingImageFilter<InternalImageType, InternalImageType> MatchingFilterType;
            MatchingFilterType::Pointer matcher = MatchingFilterType::New();

            matcher->SetInput(movingImageCaster->GetOutput());
            matcher->SetReferenceImage(fixedImageCaster->GetOutput());
            matcher->SetNumberOfHistogramLevels(histogramLevel);
            matcher->SetNumberOfMatchPoints(histogramMatchPoints);

            matcher->ThresholdAtMeanIntensityOn();

			cout << "Starting demons";

			typedef itk::Vector<float, Dimension> VectorPixelType;
            typedef itk::Image<VectorPixelType, Dimension> DisplacementFieldType;
            typedef itk::DemonsRegistrationFilter<InternalImageType, InternalImageType, DisplacementFieldType> RegistrationFilterType;
            RegistrationFilterType::Pointer filter = RegistrationFilterType::New();
            filter->SetFixedImage(fixedImageCaster->GetOutput());
            filter->SetMovingImage(matcher->GetOutput());

            filter->SetNumberOfIterations(numberOfIterations);
            filter->SetStandardDeviations(standardDeviations);

            try {
                filter->Update();
            }
            catch (itk::ExceptionObject &error) {
                std::cerr << "Error: " << error << std::endl;
                return EXIT_FAILURE;
            }

			cout << "Starting warping";

            typedef itk::WarpImageFilter<MovingImageType, MovingImageType, DisplacementFieldType> WarperType;
            typedef itk::LinearInterpolateImageFunction<MovingImageType, double> InterpolatorType;
            WarperType::Pointer warper = WarperType::New();
            InterpolatorType::Pointer interpolator = InterpolatorType::New();
            FixedImageType::Pointer fixedImage = fixedFilter->GetOutput();
            warper->SetInput(movingFilter->GetOutput());
            warper->SetInterpolator(interpolator);
            warper->SetOutputSpacing(fixedImage->GetSpacing());
            warper->SetOutputOrigin(fixedImage->GetOrigin());
            warper->SetOutputDirection(fixedImage->GetDirection());

            warper->SetDisplacementField(filter->GetOutput());
			
            /*
             * File output
             *
             */
			
            std::ostringstream oss;
            oss << "output-" << histogramLevel << "-" << histogramMatchPoints <<
                    "-" << standardDeviations << "-" << numberOfIterations << "/";
            std::string outputDirStr(oss.str());
            const char *outputDirectory = outputDirStr.c_str();

            itksys::SystemTools::MakeDirectory(outputDirectory);
			cout << "File: " << outputDirectory;

            const unsigned int OutputDimension = 2;
            typedef itk::Image<PixelType, OutputDimension> Image2DType;
            typedef itk::ImageSeriesWriter<
                    MovingImageType, Image2DType> SeriesWriterType;

            SeriesWriterType::Pointer seriesWriter = SeriesWriterType::New();
            //seriesWriter->SetInput(warper->GetOutput());
			seriesWriter->SetInput(movingImageReader->GetOutput());
            seriesWriter->SetImageIO(movingDicomIO);

            //NamesGeneratorType::Pointer outputNamesGenerator = NamesGeneratorType::New();
            movingNameGenerator->SetOutputDirectory(outputDirectory);
            seriesWriter->SetFileNames(movingNameGenerator->GetOutputFileNames());

            seriesWriter->SetMetaDataDictionaryArray(
                    movingImageReader->GetMetaDataDictionaryArray());

            try {
                seriesWriter->Update();
            }
            catch (itk::ExceptionObject &excp) {
                std::cerr << "Exception thrown while writing the series " << std::endl;
                std::cerr << excp << std::endl;
                return EXIT_FAILURE;
            }
        }
     }
    /**
    *
    * VTK
    *
    */
//
//    //typedef itk::ImageToVTKImageFilter < RGBImageType > ConnectorType;
//    typedef itk::ImageToVTKImageFilter < MovingImageType > warpConnectorType;
//    warpConnectorType::Pointer rawConnector = warpConnectorType::New();
//    //connector->SetInput( rgbFilter->GetOutput() );
//    rawConnector->SetInput( fixedFilter->GetOutput() );
//    //connector->SetInput( filter->GetOutput() );
//    //connector->SetInput( reader->GetOutput() );
//    rawConnector->Update();
//
//
//
//    warpConnectorType::Pointer movingImageConnector = warpConnectorType::New();
//    movingImageConnector->SetInput( movingFilter->GetOutput() );
//    movingImageConnector->Update();
//
//
//
//
//    //typedef itk::ImageToVTKImageFilter < MovingImageType > warpConnectorType;
//    warpConnectorType::Pointer connector = warpConnectorType::New();
//    //connector->SetInput( rgbFilter->GetOutput() );
//    connector->SetInput( checkerBoardFilter->GetOutput() );
//    //connector->SetInput( filter->GetOutput() );
//    //connector->SetInput( reader->GetOutput() );
//    connector->Update();
//
//    vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
//    vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
//
//    interactors.push_back(iren);
//    iren->SetRenderWindow(renWin);
//
//
//
//    // Create the renderer, the render window, and the interactor. The renderer
//    // draws into the render window, the interactor enables mouse- and
//    // keyboard-based interaction with the scene.
//
//    vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
//
//
//    vtkSmartPointer<vtkRenderer> rendererVolOriginal = vtkSmartPointer<vtkRenderer>::New();
//    vtkSmartPointer<vtkRenderer> rendererBinaryOutput = vtkSmartPointer<vtkRenderer>::New();
//
//
//
//    renWin->AddRenderer(rendererVolOriginal);
//    rendererVolOriginal->SetViewport(xmins[2],ymins[2],xmaxs[2],ymaxs[2]);
//
//    renWin->AddRenderer(ren);
//    ren->SetViewport(xmins[3],ymins[3],xmaxs[3],ymaxs[3]);
//
//    renWin->AddRenderer(rendererBinaryOutput);
//    rendererBinaryOutput->SetViewport(xmins[1],ymins[1],xmaxs[1],ymaxs[1]);
//
//
//
//
//
//    vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> volumeMapper =
//            vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
//    volumeMapper->SetInputData(connector->GetOutput());
//
//    vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> volumeMapperOriginal =
//            vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
//    volumeMapperOriginal->SetInputData(rawConnector->GetOutput());
//
//    vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> volumeMapperBinary =
//            vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
//    volumeMapperBinary->SetInputData(movingImageConnector->GetOutput());
//
//
//    vtkSmartPointer<vtkColorTransferFunction>volumeColor =
//         vtkSmartPointer<vtkColorTransferFunction>::New();
//       //volumeColor->AddRGBPoint(0,    0.0, 0.0, 0.0);
//       //volumeColor->AddRGBPoint(500,  1.0, 0.5, 0.3);
//      // volumeColor->AddRGBPoint(1000, 1.0, 0.5, 0.3);
//    //   volumeColor->AddRGBPoint(1150, 1.0, 1.0, 0.9);
//    volumeColor->AddRGBPoint(1150,    0.0, 0.0, 0.0);
//    volumeColor->AddRGBPoint(1000,  1.0, 0.5, 0.3);
//    volumeColor->AddRGBPoint(500, 1.0, 0.5, 0.3);
//    volumeColor->AddRGBPoint(0, 1.0, 1.0, 0.9);
//
//
//    // The opacity transfer function is used to control the opacity
//       // of different tissue types.
//    vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity =
//         vtkSmartPointer<vtkPiecewiseFunction>::New();
//    volumeScalarOpacity->AddPoint(0,    0.00);
//    volumeScalarOpacity->AddPoint(500,  0.15);
//    volumeScalarOpacity->AddPoint(1000, 0.15);
//    volumeScalarOpacity->AddPoint(1150, 0.85);
//
//    //volumeScalarOpacity->AddPoint(1150,    0.00);
//    //volumeScalarOpacity->AddPoint(1000,  0.15);
//    //volumeScalarOpacity->AddPoint(500, 0.15);
//    //volumeScalarOpacity->AddPoint(0, 0.85);
//
//       // The gradient opacity function is used to decrease the opacity
//       // in the "flat" regions of the volume while maintaining the opacity
//       // at the boundaries between tissue types.  The gradient is measured
//       // as the amount by which the intensity changes over unit distance.
//       // For most medical data, the unit distance is 1mm.
//       vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity =
//         vtkSmartPointer<vtkPiecewiseFunction>::New();
//       volumeGradientOpacity->AddPoint(0,   0.0);
//       volumeGradientOpacity->AddPoint(90,  0.5);
//       volumeGradientOpacity->AddPoint(100, 1.0);
//
//
//       vtkSmartPointer<vtkVolumeProperty> volumeProperty =
//         vtkSmartPointer<vtkVolumeProperty>::New();
//       volumeProperty->SetColor(volumeColor);
//       volumeProperty->SetScalarOpacity(volumeScalarOpacity);
//       volumeProperty->SetGradientOpacity(volumeGradientOpacity);
//       volumeProperty->SetInterpolationTypeToLinear();
//       volumeProperty->ShadeOn();
//       volumeProperty->SetAmbient(0.4);
//       volumeProperty->SetDiffuse(0.6);
//       volumeProperty->SetSpecular(0.2);
//       // The vtkVolume is a vtkProp3D (like a vtkActor) and controls the position
//       // and orientation of the volume in world coordinates.
//       vtkSmartPointer<vtkVolume> volume =
//        vtkSmartPointer<vtkVolume>::New();
//       volume->SetMapper(volumeMapper);
//       volume->SetProperty(volumeProperty);
//
//
//    vtkSmartPointer<vtkVolume> volumeOriginal =
//            vtkSmartPointer<vtkVolume>::New();
//    volumeOriginal->SetMapper(volumeMapperOriginal);
//    volumeOriginal->SetProperty(volumeProperty);
//
//
//    vtkSmartPointer<vtkVolume> volumeBinary =
//            vtkSmartPointer<vtkVolume>::New();
//    volumeBinary->SetMapper(volumeMapperBinary);
//    volumeBinary->SetProperty(volumeProperty);
//
//       // Finally, add the volume to the renderer
//     ren->SetBackground(1,1,1);
//     ren->AddViewProp(volume);
//     ren->ResetCamera();
//
//
//
//    rendererVolOriginal ->SetBackground(1,1,1);
//    rendererVolOriginal ->AddViewProp(volumeOriginal);
//    rendererVolOriginal ->ResetCamera();
//
//    rendererBinaryOutput ->SetBackground(1,1,1);
//    rendererBinaryOutput ->AddViewProp(volumeBinary);
//    rendererBinaryOutput ->ResetCamera();
//
//
//    rendererVolOriginal->SetActiveCamera(rendererBinaryOutput->GetActiveCamera());
//    ren->SetActiveCamera(rendererVolOriginal->GetActiveCamera());
//
//
//
//
//      // Increase the size of the render window
//     renWin->SetSize(600, 600);
//    renWin->Render();
//
//      // Interact with the data.
//     //iren->Initialize();
//     //iren->Start();
//
//    /**
//    *
//    * Visualize original
//    *
//    */
//
//
//    warpConnectorType::Pointer originalSliceConnector = warpConnectorType::New();
//    originalSliceConnector->SetInput( fixedFilter->GetOutput() );
//    originalSliceConnector->Update();
//
//    int *dimensions = connector->GetOutput()->GetDimensions();
//
//
//
//
//    vtkSmartPointer<vtkRenderWindow> renderWindow =
//            vtkSmartPointer<vtkRenderWindow>::New();
//
//    renderWindow->SetSize(dimensions[0] * 2, dimensions[1]);
//
//
//    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
//            vtkSmartPointer<vtkRenderWindowInteractor>::New();
//
//    vtkSmartPointer<vtkRenderer> rendererLeft =
//            vtkSmartPointer<vtkRenderer>::New();
//
//    vtkSmartPointer<vtkRenderer> rendererRight =
//            vtkSmartPointer<vtkRenderer>::New();
//
//    renderWindow->AddRenderer(rendererLeft);
//    renderWindow->AddRenderer(rendererRight);
//    rendererLeft->SetViewport(xmins2[0],ymins2[0],xmaxs2[0],ymaxs2[0]);
//    rendererRight->SetViewport(xmins2[1],ymins2[1],xmaxs2[1],ymaxs2[1]);
//
//    interactors.push_back(renderWindowInteractor);
//
//    vtkSmartPointer<CustomInteractor> customInteractorStyle =
//            vtkSmartPointer<CustomInteractor>::New();
//
//
//    renderWindowInteractor->SetRenderWindow(renderWindow);
//    renderWindow->Render();
//    renderWindowInteractor->SetInteractorStyle(customInteractorStyle );
//
//    vtkSmartPointer<vtkImageMapper> imageMapperLeft = vtkSmartPointer<vtkImageMapper>::New();
//    imageMapperLeft->SetInputData(rawConnector->GetOutput());
//
//    vtkSmartPointer<vtkImageMapper> imageMapperRight = vtkSmartPointer<vtkImageMapper>::New();
//    imageMapperRight->SetInputData(connector->GetOutput());
//
//
//    customInteractorStyle->SetMapper1(imageMapperLeft);
//    customInteractorStyle->SetMapper2(imageMapperRight);
//    customInteractorStyle->SetRenderer1(rendererLeft);
//    customInteractorStyle->SetRenderer2(rendererRight);
//    customInteractorStyle->SetRenderWindow(renderWindow);
//
//
//    //customInteractorStyle->SetStatusMapper(sliceTextMapper);
//
//    rendererRight->GetActiveCamera()->ParallelProjectionOn();
//    rendererLeft->GetActiveCamera()->ParallelProjectionOn();
//
//    vtkSmartPointer<vtkActor2D> imageActorLeft = vtkSmartPointer<vtkActor2D>::New();
//    imageActorLeft ->SetMapper(imageMapperLeft);
//    rendererLeft->AddActor2D(imageActorLeft);
//    rendererLeft->SetBackground(1,1,1);
//
//    vtkSmartPointer<vtkActor2D> imageActorRight = vtkSmartPointer<vtkActor2D>::New();
//    imageActorRight ->SetMapper(imageMapperRight);
//    rendererRight->AddActor2D(imageActorRight);
//
//    rendererLeft->Render();
//    rendererLeft->ResetCamera();
//    rendererLeft->Render();
//
//    rendererRight->Render();
//    rendererRight->ResetCamera();
//    rendererRight->Render();
//
//
//
//    interactors[1]->Start();
//
//
//
//
	getchar();
    return 0;
}


SmoothingFilterType::Pointer Smoothing(FixedImageReaderType::Pointer reader){

    SmoothingFilterType::Pointer filter = SmoothingFilterType::New();

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
        exit(-1);
    }

    return filter;

}
