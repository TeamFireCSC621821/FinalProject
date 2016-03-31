
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageToVTKImageFilter.h"
#include "vtkImageViewer.h"
#include "vtkRenderWindowInteractor.h"

#include <iostream>


using namespace std;

int main(int argc, char **argv) {
    typedef itk::Image<unsigned short,2> ImageType;
    typedef itk::ImageFileReader<ImageType> ReaderType;
    typedef itk::ImageToVTKImageFilter < ImageType> ConnectorType;
    ReaderType::Pointer reader = ReaderType::New();
    ConnectorType::Pointer connector = ConnectorType::New();

    reader->SetFileName( "~/mic.jpg" );
    connector->SetInput( reader->GetOutput() );

    vtkImageViewer * viewer = vtkImageViewer::New();
    vtkRenderWindowInteractor * renderWindowInteractor = vtkRenderWindowInteractor::New();

    viewer->SetupInteractor( renderWindowInteractor );
    viewer->SetInputData(connector->GetOutput());
    viewer->Render();
    viewer->SetColorWindow( 20 );
    viewer->SetColorLevel( 0 );
    renderWindowInteractor->Start();


    return 0;
}