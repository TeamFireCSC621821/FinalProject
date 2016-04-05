
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageToVTKImageFilter.h"
#include "vtkImageViewer.h"
#include "vtkRenderWindowInteractor.h"
#include "itkCurvatureFlowImageFilter.h"

//For Edge Preserving Smoothing
#include "itkGradientAnisotropicDiffusionImageFilter.h"

//Threshold Segmentation
#include "itkThresholdSegmentationLevelSetImageFilter.h"
#include "itkFastMarchingImageFilter.h"
#include "itkShapeDetectionLevelSetImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"




using namespace std;

int main(int argc, char **argv) {
    //typedef itk::Image<unsigned short,2> ImageType;
    //typedef itk::Image< float , 2 > ImageType;
    //typedef itk::CurvatureFlowImageFilter<ImageType, ImageType> SmoothingFilterType;

    //Numbers to play with
    int seedX = 200;
    int seedY = 100;
    int lowerThreshold = 0;
    int upperThreshold = 260;
    const double initialDistance = 20;

    typedef float InputPixelType;
    typedef unsigned int                            OutputPixelType;
    typedef itk::Image< OutputPixelType, 2 > OutputImageType;
    typedef itk::Image< InputPixelType, 2 > InputImageType;



    typedef itk::GradientAnisotropicDiffusionImageFilter<InputImageType, InputImageType > FilterType;
    FilterType::Pointer filter = FilterType::New();


    typedef itk::ThresholdSegmentationLevelSetImageFilter< InputImageType,
            InputImageType > ThresholdSegmentationLevelSetImageFilterType;

    ThresholdSegmentationLevelSetImageFilterType::Pointer thresholdSegmentation =
            ThresholdSegmentationLevelSetImageFilterType::New();

    typedef itk::BinaryThresholdImageFilter< InputImageType, OutputImageType > ThresholdingFilterType;
    ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();

    typedef itk::FastMarchingImageFilter< InputImageType, InputImageType > FastMarchingFilterType;

    FastMarchingFilterType::Pointer fastMarching = FastMarchingFilterType::New();


    thresholder->SetLowerThreshold( -1000.0 );
    thresholder->SetUpperThreshold( 0.0 );
    thresholder->SetOutsideValue( 0 );
    thresholder->SetInsideValue( 255 );



    typedef itk::ImageFileReader<InputImageType> ReaderType;
    typedef itk::ImageToVTKImageFilter < OutputImageType > ConnectorType;
    ReaderType::Pointer reader = ReaderType::New();
    ConnectorType::Pointer connector = ConnectorType::New();

    reader->SetFileName(argv[1] );
    try
    {
        reader->Update();
    }
    catch (itk::ExceptionObject &ex)
    {
        std::cout << ex << std::endl;
    }

    filter->SetInput( reader->GetOutput() );
    filter->SetNumberOfIterations( 5 );
    filter->SetTimeStep( 0.125 );
    filter->SetConductanceParameter( 3.0 );
    filter->Update();







    thresholdSegmentation->SetPropagationScaling( 1.0 );
    thresholdSegmentation->SetCurvatureScaling( 1.0 );

    //  Software Guide : EndCodeSnippet
    //  The level set solver will stop if the convergence criteria has been
    //  reached or if the maximum number of iterations has elasped.  The
    //  convergence criteria is defined in terms of the root mean squared (RMS)
    //  change in the level set function. When RMS change for an iteration is
    //  below a user-specified threshold, the solution is considered to have
    //  converged.
    thresholdSegmentation->SetMaximumRMSError( 0.02 );
    thresholdSegmentation->SetNumberOfIterations( 1200 );
    //    thresholdSegmentation->SetMaximumRMSError( atof(argv[8]) );
    //    thresholdSegmentation->SetNumberOfIterations( atoi(argv[9]) );
    // Software Guide : BeginLatex
    //
    // The convergence criteria \code{MaximumRMSError} and
    // \code{MaximumIterations} are set as in previous examples.  We now set
    // the upper and lower threshold values $U$ and $L$, and the isosurface
    // value to use in the initial model.
    //
    // Software Guide : EndLatex
    // Software Guide : BeginCodeSnippet

    thresholdSegmentation->SetUpperThreshold( upperThreshold);
    thresholdSegmentation->SetLowerThreshold(lowerThreshold);
    thresholdSegmentation->SetIsoSurfaceValue(0.0);
    // Software Guide : EndCodeSnippet
    //  Software Guide : BeginLatex
    //
    //  The filters are now connected in a pipeline indicated in
    //  Figure~\ref{fig:ThresholdSegmentationLevelSetImageFilterDiagram}.
    //  Remember that before calling \code{Update()} on the file writer object,
    //  the fast marching filter must be initialized with the seed points and
    //  the output from the reader object.  See previous examples and the
    //  source code for this section for details.
    //
    //  Software Guide : EndLatex
    // Software Guide : BeginCodeSnippet
    thresholdSegmentation->SetInput( fastMarching->GetOutput() );
    thresholdSegmentation->SetFeatureImage( filter->GetOutput() );
    thresholdSegmentation->Update();

    thresholder->SetInput( thresholdSegmentation->GetOutput() );


    // Software Guide : EndCodeSnippet
    //
    //  The FastMarchingImageFilter requires the user to provide a seed
    //  point from which the level set will be generated. The user can actually
    //  pass not only one seed point but a set of them. Note the the
    //  FastMarchingImageFilter is used here only as a helper in the
    //  determination of an initial Level Set. We could have used the
    //  \doxygen{DanielssonDistanceMapImageFilter} in the same way.
    //
    //  The seeds are passed stored in a container. The type of this
    //  container is defined as \code{NodeContainer} among the
    //  FastMarchingImageFilter traits.
    //
    typedef FastMarchingFilterType::NodeContainer           NodeContainer;
    typedef FastMarchingFilterType::NodeType                NodeType;
    NodeContainer::Pointer seeds = NodeContainer::New();
    //InternalImageType::IndexType  seedPosition;
    itk::LevelSetNode<float, 2>::IndexType seedPosition;
    seedPosition[0] = seedX ;
    seedPosition[1] = seedY;
    //  Nodes are created as stack variables and initialized with a value and an
    //  \doxygen{Index} position. Note that here we assign the value of minus the
    //  user-provided distance to the unique node of the seeds passed to the
    //  FastMarchingImageFilter. In this way, the value will increment
    //  as the front is propagated, until it reaches the zero value corresponding
    //  to the contour. After this, the front will continue propagating until it
    //  fills up the entire image. The initial distance is taken here from the
    //  command line arguments. The rule of thumb for the user is to select this
    //  value as the distance from the seed points at which he want the initial
    //  contour to be.

    NodeType node;
    const double seedValue = - initialDistance;
    node.SetValue( seedValue );
    node.SetIndex( seedPosition );
    //
    //  The list of nodes is initialized and then every node is inserted using
    //  the \code{InsertElement()}.
    seeds->Initialize();
    seeds->InsertElement( 0, node );
    //  The set of seed nodes is passed now to the
    //  FastMarchingImageFilter with the method
    //  \code{SetTrialPoints()}.
    fastMarching->SetTrialPoints(  seeds  );
    //
    //  Since the FastMarchingImageFilter is used here just as a
    //  Distance Map generator. It does not require a speed image as input.
    //  Instead the constant value $1.0$ is passed using the
    //  \code{SetSpeedConstant()} method.
    fastMarching->SetSpeedConstant( 1.0 );
    fastMarching->Update();
    //  The FastMarchingImageFilter requires the user to specify the size of the
    //  image to be produced as output. This is done using the
    //  \code{SetOutputRegion()} method. Note that the size is obtained here from
    //  the output image of the smoothing filter. The size of this image is valid
    //  only after the \code{Update()} methods of this filter has been called
    //  directly or indirectly. Other image parameters such as Origin, Spacing
    //  and Direction are set in a similar manner.
    //  Software Guide : BeginLatex
    //
    //  Invoking the \code{Update()} method on the writer triggers the
    //  execution of the pipeline.  As usual, the call is placed in a
    //  \code{try/catch} block should any errors occur or exceptions be thrown.
    //
    //  Software Guide : EndLatex
    // Software Guide : BeginCodeSnippet
    try
    {


        fastMarching->SetOutputRegion( filter->GetOutput()->GetBufferedRegion() );
        fastMarching->SetOutputSpacing( filter->GetOutput()->GetSpacing() );
        fastMarching->SetOutputOrigin( filter->GetOutput()->GetOrigin() );
        fastMarching->SetOutputDirection( filter->GetOutput()->GetDirection() );
        fastMarching->Update();
        thresholdSegmentation->Update();
        thresholder->Update();


    }
    catch( itk::ExceptionObject & excep )
    {
        std::cerr << "Exception caught !" << std::endl;
        std::cerr << excep << std::endl;
        return EXIT_FAILURE;
    }





    connector->SetInput( thresholder->GetOutput() );
    connector->Update();

    vtkImageViewer * viewer = vtkImageViewer::New();
    vtkRenderWindowInteractor * renderWindowInteractor = vtkRenderWindowInteractor::New();

    viewer->SetupInteractor( renderWindowInteractor );
    viewer->SetInputData(connector->GetOutput());

    // Print out some useful information
    std::cout << std::endl;
    std::cout << "Max. no. iterations: " << thresholdSegmentation->GetNumberOfIterations() << std::endl;
    std::cout << "Max. RMS error: " << thresholdSegmentation->GetMaximumRMSError() << std::endl;
    std::cout << std::endl;
    std::cout << "No. elpased iterations: " << thresholdSegmentation->GetElapsedIterations() << std::endl;
    std::cout << "RMS change: " << thresholdSegmentation->GetRMSChange() << std::endl;


    viewer->Render();
    viewer->SetColorWindow( 255 );
    viewer->SetColorLevel( 128 );
    renderWindowInteractor->Start();




    return 0;
}