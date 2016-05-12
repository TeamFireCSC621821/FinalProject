
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

	typedef itk::GDCMSeriesFileNames NamesGeneratorType;
    NamesGeneratorType::Pointer fixedNameGenerator = NamesGeneratorType::New();
    NamesGeneratorType::Pointer movingNameGenerator = NamesGeneratorType::New();


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
				cout << filter->GetMetric() << endl;
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

			cout << outputDirectory << " " << filter->GetMetric() << endl;
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
