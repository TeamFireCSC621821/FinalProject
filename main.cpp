
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

#include <iostream>


using namespace std;

int main(int argc, char **argv) {
    //typedef itk::Image<unsigned short,2> ImageType;
    //typedef itk::Image< float , 2 > ImageType;
    //typedef itk::CurvatureFlowImageFilter<ImageType, ImageType> SmoothingFilterType;

    typedef float InputPixelType;
    typedef float OutputPixelType;
    typedef itk::Image< InputPixelType, 2 > InputImageType;
    typedef itk::Image< OutputPixelType, 2 > OutputImageType;


    typedef itk::GradientAnisotropicDiffusionImageFilter<InputImageType, OutputImageType > FilterType;
    FilterType::Pointer filter = FilterType::New();




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


    connector->SetInput( filter->GetOutput() );
    connector->Update();

    vtkImageViewer * viewer = vtkImageViewer::New();
    vtkRenderWindowInteractor * renderWindowInteractor = vtkRenderWindowInteractor::New();

    viewer->SetupInteractor( renderWindowInteractor );
    viewer->SetInputData(connector->GetOutput());




    viewer->Render();
    viewer->SetColorWindow( 255 );
    viewer->SetColorLevel( 128 );
    renderWindowInteractor->Start();


    return 0;
}