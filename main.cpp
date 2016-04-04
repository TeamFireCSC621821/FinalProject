
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageToVTKImageFilter.h"
#include "vtkImageViewer.h"
#include "vtkRenderWindowInteractor.h"
#include "itkCurvatureFlowImageFilter.h"

#include <iostream>


using namespace std;

int main(int argc, char **argv) {
    //typedef itk::Image<unsigned short,2> ImageType;
    typedef itk::Image< float , 2 > ImageType;
    typedef itk::CurvatureFlowImageFilter<ImageType, ImageType> SmoothingFilterType;
    SmoothingFilterType::Pointer smoother = SmoothingFilterType::New();

    typedef itk::ImageFileReader<ImageType> ReaderType;
    typedef itk::ImageToVTKImageFilter < ImageType> ConnectorType;
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

    smoother->SetInput( reader->GetOutput() );
    smoother->SetNumberOfIterations(30 );
    smoother->SetTimeStep( 0.1 );
    smoother->Update();




    connector->SetInput( smoother->GetOutput() );
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