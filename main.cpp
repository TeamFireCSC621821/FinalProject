/*!
* @brief The main program. Reads fixed and moving dicom directories from the command line and produces a registered image.
* @file
* */
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

const unsigned int      Dimension = 3; //! the pixel dimension

//registration
typedef float PixelType; //! typedef of fixed and moving image pixels
typedef itk::Image< PixelType, Dimension > FixedImageType; //! typedef of fixed itk image
typedef itk::Image< PixelType, Dimension > MovingImageType; //! typedef of moving itk image
 
typedef itk::ImageSeriesReader< FixedImageType >        FixedImageReaderType; //! typedef of fixed image reader
typedef itk::ImageSeriesReader< MovingImageType >        MovingImageReaderType; //! typedef of moving image reader

typedef float InternalPixelType; //! typedef of internal pixels for registration
typedef itk::Image<InternalPixelType, Dimension> InternalImageType; //! typedef of internal image for registration
typedef itk::Vector<float, Dimension> VectorPixelType; //! typedef of displacement field vectors
typedef itk::Image<VectorPixelType, Dimension> DisplacementFieldType; //! typedef of displacement field
typedef itk::DemonsRegistrationFilter<InternalImageType, InternalImageType, DisplacementFieldType> RegistrationFilterType; //! typedef of the demons registration filter

typedef itk::GradientAnisotropicDiffusionImageFilter<FixedImageType,FixedImageType > SmoothingFilterType; //! typedef of the smoothing preprocessing filter

//! This function preprocesses an image before registration steps occur. Returns a filter reader to output.
SmoothingFilterType::Pointer Smoothing(FixedImageReaderType::Pointer);

//! This function does all processing
int process(char**argv);//char*fixedname, char*movingname);

//! Class to receive callbacks during registration iterations and print metrics.
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
	ofstream * os;	//! output stream to print metrics to

public:

	//! Sets the output stream
	void SetStream(ofstream & ostream) {
		os = &ostream;
	}

	void Execute(itk::Object *caller, const itk::EventObject & event)
	{
		Execute((const itk::Object *)caller, event);
	}

	//! callback prints to the output stream the metric for the current iteration
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
	return process(argv);
}


SmoothingFilterType::Pointer Smoothing(FixedImageReaderType::Pointer reader){

	//! define a smoothing filter with some parameters and input from the reader
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

int process(char **argv) {
	//! This is for image loading
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

	fixedNameGenerator->SetDirectory(argv[1]);
	movingNameGenerator->SetDirectory(argv[2]);



	//! This is for fixedImage loading
	typedef std::vector< std::string >    SeriesIdContainer;
	const SeriesIdContainer & fixedSeriesUID = fixedNameGenerator->GetSeriesUIDs();
	std::cout << fixedSeriesUID.size() << std::endl;
	SeriesIdContainer::const_iterator fixedSeriesItr = fixedSeriesUID.begin();
	SeriesIdContainer::const_iterator fixedSeriesEnd = fixedSeriesUID.end();
	while (fixedSeriesItr != fixedSeriesEnd)
	{
		std::cout << fixedSeriesItr->c_str() << std::endl;
		fixedSeriesItr++;
	}
	std::string fixedSeriesIdentifier;
	fixedSeriesIdentifier = fixedSeriesUID.begin()->c_str();
	std::cout << fixedSeriesIdentifier.c_str() << std::endl;

	typedef std::vector< std::string >   FileNamesContainer;
	FileNamesContainer fixedFileNames;
	fixedFileNames = fixedNameGenerator->GetFileNames(fixedSeriesIdentifier);

	fixedImageReader->SetFileNames(fixedFileNames);
	try
	{
		fixedImageReader->Update();
	}
	catch (itk::ExceptionObject &ex)
	{
		std::cout << ex << std::endl;
	}

	//! This is for moving image loading
	const SeriesIdContainer & movingSeriesUID = movingNameGenerator->GetSeriesUIDs();
	std::cout << movingSeriesUID.size() << std::endl;
	SeriesIdContainer::const_iterator movingSeriesItr = movingSeriesUID.begin();
	SeriesIdContainer::const_iterator movingSeriesEnd = movingSeriesUID.end();
	while (movingSeriesItr != movingSeriesEnd)
	{
		std::cout << movingSeriesItr->c_str() << std::endl;
		movingSeriesItr++;
	}
	std::string movingSeriesIdentifier;
	movingSeriesIdentifier = movingSeriesUID.begin()->c_str();
	std::cout << movingSeriesIdentifier.c_str() << std::endl;

	FileNamesContainer movingFileNames;
	movingFileNames = movingNameGenerator->GetFileNames(movingSeriesIdentifier);

	movingImageReader->SetFileNames(movingFileNames);
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

	/*!
	* Thirion's Demon Registration
	* Starting at 512 histogram levels, 6 match points, and 0.5 standard deviations
	*/
	int histogramLevel = 512;
	int histogramMatchPoints = 6;
	int numberOfIterations = 50;
	float standardDeviations = 0.5;

	typedef itk::CastImageFilter<FixedImageType, InternalImageType> FixedImageCasterType;
	typedef itk::CastImageFilter<MovingImageType, InternalImageType> MovingImageCasterType;

	FixedImageCasterType::Pointer fixedImageCaster = FixedImageCasterType::New();
	MovingImageCasterType::Pointer movingImageCaster = MovingImageCasterType::New();

	fixedImageCaster->SetInput(fixedFilter->GetOutput());
	movingImageCaster->SetInput(movingFilter->GetOutput());

	ofstream metricsFile;
	metricsFile.open("metrics.csv");

	for (histogramLevel = 512; histogramLevel <= 2048; histogramLevel *= 2) {
		for (histogramMatchPoints = 6; histogramMatchPoints <= 8; histogramMatchPoints++) {
			for (standardDeviations = 0.5; standardDeviations <= 1.5; standardDeviations += 0.5) {

				metricsFile << "Histogram Levels, Histogram Match Points, Standard Deviations\n";
				metricsFile << histogramLevel << "," << histogramMatchPoints << "," << standardDeviations << endl;
				metricsFile << "Iteration,Mean Square Difference\n";
				metricsFile << " ," << histogramLevel << "-" << histogramMatchPoints << "-" << standardDeviations << endl;

				//! Histogram matching
				cout << "Starting Histogram Matching\n";

				typedef itk::HistogramMatchingImageFilter<InternalImageType, InternalImageType> MatchingFilterType;
				MatchingFilterType::Pointer matcher = MatchingFilterType::New();

				matcher->SetInput(movingImageCaster->GetOutput());
				matcher->SetReferenceImage(fixedImageCaster->GetOutput());
				matcher->SetNumberOfHistogramLevels(histogramLevel);
				matcher->SetNumberOfMatchPoints(histogramMatchPoints);

				matcher->ThresholdAtMeanIntensityOn();

				//! Thririon's demons registration
				cout << "Starting demons\n";

				RegistrationFilterType::Pointer filter = RegistrationFilterType::New();
				CommandIterationFileout::Pointer observer = CommandIterationFileout::New();
				observer->SetStream(metricsFile);
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

				//! Warping moving image
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

				/*!
				* File output
				* Writing registered image to a new dicom directory
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
				seriesWriter->SetInput(warper->GetOutput());
				seriesWriter->SetImageIO(movingDicomIO);

				fixedNameGenerator->SetOutputDirectory(outputDirectory);
				seriesWriter->SetFileNames(fixedNameGenerator->GetOutputFileNames());

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
