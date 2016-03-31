
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageToVTKImageFilter.h"
#include "vtkImageViewer.h"
#include "vtkRenderWindowInteractor.h"
#include "itkCurvatureFlowImageFilter.h"

#include <iostream>


using namespace std;

int main(int argc, char **argv) {
    typedef itk::Image<unsigned short,2> ImageType;
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
    connector->SetInput( reader->GetOutput() );
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