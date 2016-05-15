
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageSeriesReader.h"
#include "itkCurvatureFlowImageFilter.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkGDCMImageIO.h"
#include "itkMacro.h"

#include "itkCheckerBoardImageFilter.h"
#include "itkImageIterator.h"

//For Registration
#include "itkDemonsRegistrationFilter.h"
#include "itkHistogramMatchingImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkWarpImageFilter.h"

//For Edge Preserving Smoothing
#include "itkGradientAnisotropicDiffusionImageFilter.h"

//For File out
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

//registration
//const unsigned int Dimension = 3;
typedef float PixelType;
typedef itk::Image< PixelType, Dimension > FixedImageType;
typedef itk::Image< PixelType, Dimension > MovingImageType;

typedef itk::ImageSeriesReader< FixedImageType >        FixedImageReaderType;
typedef itk::ImageSeriesReader< MovingImageType >        MovingImageReaderType;

typedef float InternalPixelType;
typedef itk::Image<InternalPixelType, Dimension> InternalImageType;
typedef itk::Vector<float, Dimension> VectorPixelType;
typedef itk::Image<VectorPixelType, Dimension> DisplacementFieldType;
typedef itk::DemonsRegistrationFilter<InternalImageType, InternalImageType, DisplacementFieldType> RegistrationFilterType;

typedef itk::GradientAnisotropicDiffusionImageFilter<FixedImageType,FixedImageType > SmoothingFilterType;

//checkerboard typedefs
typedef itk::CheckerBoardImageFilter< FixedImageType > CheckerBoardFilterType;


SmoothingFilterType::Pointer Smoothing(FixedImageReaderType::Pointer);

class CommandIterationFileout : public itk::Command
{
public:
	typedef  CommandIterationFileout                     Self;
	typedef  itk::Command                               Superclass;
	typedef  itk::SmartPointer<CommandIterationFileout>  Pointer;
	itkNewMacro(CommandIterationFileout);
protected:
	CommandIterationFileout() {};
private:
	ofstream * os;

public:

	void SetFile(ofstream & ostream) {
		os = &ostream;
	}

	void Execute(itk::Object *caller, const itk::EventObject & event)
	{
		Execute((const itk::Object *)caller, event);
	}
	void Execute(const itk::Object * object, const itk::EventObject & event)
	{
		const RegistrationFilterType * filter = static_cast< const RegistrationFilterType * >(object);
		if (!(itk::IterationEvent().CheckEvent(&event)))
		{
			return;
		}
		(*os) << filter->GetElapsedIterations() << "," <<  filter->GetMetric() << endl;
	}
};

int main(int argc, char **argv) {
	
	//This is for fixedImage loading

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


	fixedNameGenerator->SetDirectory("C:/FixedCT");
    movingNameGenerator->SetDirectory("C:/MovingCT");


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
    int numberOfIterations = 50;
    float standardDeviations = 1.0;

    typedef itk::CastImageFilter<FixedImageType, InternalImageType> FixedImageCasterType;
    typedef itk::CastImageFilter<MovingImageType, InternalImageType> MovingImageCasterType;

    FixedImageCasterType::Pointer fixedImageCaster = FixedImageCasterType::New();
    MovingImageCasterType::Pointer movingImageCaster = MovingImageCasterType::New();

    fixedImageCaster->SetInput(fixedFilter->GetOutput());
    movingImageCaster->SetInput(movingFilter->GetOutput());

	ofstream metricsFile;
	metricsFile.open("metrics.csv");

	for(histogramLevel = 512; histogramLevel <= 2048; histogramLevel*=2) {
		for (histogramMatchPoints = 6; histogramMatchPoints <= 8; histogramMatchPoints++) {
			for (standardDeviations = 0.5; standardDeviations <= 1.5; standardDeviations += 0.5) {
	
				metricsFile << "Histogram Levels, Histogram Match Points, Standard Deviations\n";
				metricsFile << histogramLevel << "," << histogramMatchPoints << "," << standardDeviations << endl;
				metricsFile << "Iteration,Mean Square Difference\n";
				metricsFile << " ," << histogramLevel << "-" << histogramMatchPoints << "-" << standardDeviations << endl;

				cout << "Starting Histogram Matching\n";

				typedef itk::HistogramMatchingImageFilter<InternalImageType, InternalImageType> MatchingFilterType;
				MatchingFilterType::Pointer matcher = MatchingFilterType::New();

				matcher->SetInput(movingImageCaster->GetOutput());
				matcher->SetReferenceImage(fixedImageCaster->GetOutput());
				matcher->SetNumberOfHistogramLevels(histogramLevel);
				matcher->SetNumberOfMatchPoints(histogramMatchPoints);

				matcher->ThresholdAtMeanIntensityOn();

				cout << "Starting demons\n";

				RegistrationFilterType::Pointer filter = RegistrationFilterType::New();
				CommandIterationFileout::Pointer observer = CommandIterationFileout::New();
				observer->SetFile(metricsFile);
				filter->AddObserver(itk::IterationEvent(), observer);

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

				cout << "Starting warping\n";

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
				cout << "File: " << outputDirectory << endl;

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
				metricsFile << endl;
			}
		}
     }
	metricsFile.close();

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
