
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

#include <iostream>

#include <itkAbsoluteValueDifferenceImageFilter.h>
#include "itkIdentityTransform.h"
#include "itkResampleImageFilter.h"

using namespace std;

//! Define viewport ranges
double xmins[4] = {0,.5,0,.5};
double xmaxs[4] = {0.5,1,0.5,1};
double ymins[4] = {0,0,.5,.5};
double ymaxs[4]= {0.5,0.5,1,1};

//! Define viewport ranges
double xmins2[2] = {0,.5};
double xmaxs2[2] = {0.5,1};
double ymins2[2] = {0,0};
double ymaxs2[2]= {1,1};


const unsigned int      Dimension = 3;//! The pixel dimension
typedef float InputPixelType; //! typedef of input pixel
typedef float OutputPixelType; //! typedef of output pixel
typedef float PixelType; //! typedef of all other pixels
typedef itk::Image< PixelType, Dimension > FixedImageType; //! typedef for fixed image
typedef itk::Image< PixelType, Dimension > MovingImageType;//! typedef for moving image

typedef itk::ImageSeriesReader< FixedImageType >        FixedImageReaderType;//! typedef for fixed image reader
typedef itk::ImageSeriesReader< MovingImageType >        MovingImageReaderType;//! typedef for moving image reader
typedef itk::ImageSeriesReader< MovingImageType >        RegisteredImageReaderType;//! typedef for registered image reader
typedef itk::ImageToVTKImageFilter < FixedImageType > ConnectorType; //! typedef of connector from vtk to itk

typedef itk::CheckerBoardImageFilter< FixedImageType > CheckerBoardFilterType;//!checkerboard typedefs


//! helper class to format slice status message
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
        _Slice = _MinSlice;
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
            _ImageMapperRight->SetZSlice(_Slice);
            _ImageMapperLeft->SetZSlice(_Slice);
            _ImageRendererLeft->Render();
            _ImageRendererRight->Render();
            _RenderWindow->Render();
            std::string msg = StatusMessage::Format(_Slice, _MaxSlice);

        }
    }

    void MoveSliceBackward() {
        if(_Slice > _MinSlice) {
            _Slice -= 1;
            cout << "MoveSliceBackward::Slice = " << _Slice << std::endl;
            _ImageMapperLeft->SetZSlice(_Slice);
            _ImageMapperRight->SetZSlice(_Slice);
            _RenderWindow->Render();
        }
    }

    virtual void OnKeyDown() {
        std::string key = this->GetInteractor()->GetKeySym();
        if(key.compare("Up") == 0) {
            MoveSliceForward();
        }
        else if(key.compare("Down") == 0) {
            MoveSliceBackward();
        }
        //! forward event
        vtkInteractorStyleImage::OnKeyDown();
    }


    virtual void OnMouseWheelForward() {
        MoveSliceForward();
        //! don't forward events, otherwise the image will be zoomed
        //! in case another interactorstyle is used (e.g. trackballstyle, ...)
    }


    virtual void OnMouseWheelBackward() {
        if(_Slice > _MinSlice) {
            MoveSliceBackward();
        }
        //! don't forward events, otherwise the image will be zoomed
        //! in case another interactorstyle is used (e.g. trackballstyle, ...)
    }
};

vtkStandardNewMacro( CustomInteractor);


int main(int argc, char **argv) {
    //This is for fixedImage loading

    typedef itk::GDCMImageIO       ImageIOType;
    ImageIOType::Pointer fixedDicomIO = ImageIOType::New();
    ImageIOType::Pointer movingDicomIO = ImageIOType::New();
    ImageIOType::Pointer registeredDicomIO = ImageIOType::New();

    FixedImageReaderType::Pointer fixedImageReader = FixedImageReaderType::New();
    MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();
    RegisteredImageReaderType::Pointer registeredImageReader = RegisteredImageReaderType::New();

    fixedImageReader->SetImageIO(fixedDicomIO);
    movingImageReader->SetImageIO(movingDicomIO);
    registeredImageReader->SetImageIO(registeredDicomIO);

    //reader->SetImageIO( dicomIO );
    typedef itk::GDCMSeriesFileNames NamesGeneratorType;
    NamesGeneratorType::Pointer fixedNameGenerator = NamesGeneratorType::New();
    NamesGeneratorType::Pointer movingNameGenerator = NamesGeneratorType::New();
    NamesGeneratorType::Pointer registeredNameGenerator = NamesGeneratorType::New();

    fixedNameGenerator->SetDirectory( argv[1]);
    movingNameGenerator->SetDirectory(argv[2]);
	registeredNameGenerator->SetDirectory(argv[3]);

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


    // registered image
    const SeriesIdContainer & registeredSeriesUID = registeredNameGenerator->GetSeriesUIDs();
    std::cout << registeredSeriesUID.size() << std::endl;
    SeriesIdContainer::const_iterator registeredSeriesItr = registeredSeriesUID.begin();
    SeriesIdContainer::const_iterator registeredSeriesEnd = registeredSeriesUID.end();
    while( registeredSeriesItr != registeredSeriesEnd )
    {
        std::cout << registeredSeriesItr->c_str() << std::endl;
        registeredSeriesItr++;
    }
    std::string registeredSeriesIdentifier;
    registeredSeriesIdentifier = registeredSeriesUID.begin()->c_str();
    std::cout << registeredSeriesIdentifier.c_str() << std::endl;

    FileNamesContainer registeredFileNames;
    registeredFileNames = registeredNameGenerator->GetFileNames( registeredSeriesIdentifier );

    registeredImageReader->SetFileNames( registeredFileNames );
    try
    {
        registeredImageReader->Update();
    }
    catch (itk::ExceptionObject &ex)
    {
        std::cout << ex << std::endl;
    }


	FixedImageType::SizeType inputSize = fixedImageReader->GetOutput()->GetLargestPossibleRegion().GetSize();
	FixedImageType::SizeType outputSize = registeredImageReader->GetOutput()->GetLargestPossibleRegion().GetSize();
	
	FixedImageType::SpacingType outputSpacing;
	outputSpacing = registeredImageReader->GetOutput()->GetSpacing();

	/*!
	* Resize the image in order to fit to checkerboard and difference images
	*/

	typedef itk::IdentityTransform<double, Dimension> TransformType;
	typedef itk::ResampleImageFilter<FixedImageType, FixedImageType> ResampleImageFilterType;
	ResampleImageFilterType::Pointer resample = ResampleImageFilterType::New();
	resample->SetInput(fixedImageReader->GetOutput());
	resample->SetSize(outputSize);
	resample->SetOutputOrigin(registeredImageReader->GetOutput()->GetOrigin());
	resample->SetOutputDirection(registeredImageReader->GetOutput()->GetDirection());
	resample->SetDefaultPixelValue(100);
	resample->SetOutputSpacing(outputSpacing);
	resample->SetTransform(TransformType::New());
	resample->UpdateLargestPossibleRegion();

    /*!
    * CheckerBoard
    *
    */
	
    CheckerBoardFilterType::Pointer checkerBoardFilter = CheckerBoardFilterType::New();
	checkerBoardFilter->SetInput1(resample->GetOutput());
    checkerBoardFilter->SetInput2(registeredImageReader->GetOutput());
    checkerBoardFilter->Update();
	
    /*!
     * Difference Image
     *
     */
	cout << "starting difference Image\n";
    typedef itk::AbsoluteValueDifferenceImageFilter <FixedImageType, FixedImageType ,
            MovingImageType>
            AbsoluteValueDifferenceImageFilterType;
    AbsoluteValueDifferenceImageFilterType::Pointer absoluteValueDifferenceFilter
            = AbsoluteValueDifferenceImageFilterType::New ();
    absoluteValueDifferenceFilter->SetInput1(resample->GetOutput());
    absoluteValueDifferenceFilter->SetInput2(registeredImageReader->GetOutput());
	
    absoluteValueDifferenceFilter->Update();
	cout << "after differenece update\n";

	/*!
    *
    * VTK
    *
    */

	
	std::vector<vtkSmartPointer<vtkRenderWindowInteractor> > interactors;

    typedef itk::ImageToVTKImageFilter < MovingImageType > ImageConnectorType;

    ImageConnectorType::Pointer checkerConnector = ImageConnectorType::New();
    checkerConnector->SetInput( checkerBoardFilter->GetOutput() );
    checkerConnector->Update();

    ImageConnectorType::Pointer movingImageConnector = ImageConnectorType::New();
    movingImageConnector->SetInput( movingImageReader->GetOutput() );
    movingImageConnector->Update();

    ImageConnectorType::Pointer differenceConnector = ImageConnectorType::New();
    differenceConnector->SetInput( absoluteValueDifferenceFilter->GetOutput() );
    differenceConnector->Update();

    ImageConnectorType::Pointer fixedImageConnector = ImageConnectorType::New();
    fixedImageConnector->SetInput( fixedImageReader->GetOutput() );
    fixedImageConnector->Update();

    vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();

    interactors.push_back(iren);
    iren->SetRenderWindow(renWin);



    //! Create the renderer, the render window, and the interactor. The renderer
    //! draws into the render window, the interactor enables mouse- and
    //! keyboard-based interaction with the scene.

    vtkSmartPointer<vtkRenderer> rendererDifference = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderer> rendererCheckerboard = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderer> rendererFixedImage = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderer> rendererMovingImage = vtkSmartPointer<vtkRenderer>::New();



    renWin->AddRenderer(rendererFixedImage);
    rendererFixedImage->SetViewport(xmins[2],ymins[2],xmaxs[2],ymaxs[2]);

    renWin->AddRenderer(rendererCheckerboard);
    rendererCheckerboard->SetViewport(xmins[3],ymins[3],xmaxs[3],ymaxs[3]);

    renWin->AddRenderer(rendererMovingImage);
    rendererMovingImage->SetViewport(xmins[1],ymins[1],xmaxs[1],ymaxs[1]);

    renWin->AddRenderer(rendererDifference);
    rendererCheckerboard->SetViewport(xmins[0],ymins[0],xmaxs[0],ymaxs[0]);


    vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> volumeMapperDifference =
            vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
    volumeMapperDifference->SetInputData(differenceConnector->GetOutput());

    vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> volumeMapperChecker =
            vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
    volumeMapperChecker->SetInputData(checkerConnector->GetOutput());

    vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> volumeMapperMoving =
            vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
    volumeMapperMoving->SetInputData(movingImageConnector->GetOutput());

    vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> volumeMapperFixed =
            vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
    volumeMapperFixed->SetInputData(fixedImageConnector->GetOutput());

    vtkSmartPointer<vtkColorTransferFunction>volumeColor =
         vtkSmartPointer<vtkColorTransferFunction>::New();

    volumeColor->AddRGBPoint(1150,    0.0, 0.0, 0.0);
    volumeColor->AddRGBPoint(1000,  1.0, 0.5, 0.3);
    volumeColor->AddRGBPoint(500, 1.0, 0.5, 0.3);
    volumeColor->AddRGBPoint(0, 1.0, 1.0, 0.9);


    //! The opacity transfer function is used to control the opacity
       // of different tissue types.
    vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity =
         vtkSmartPointer<vtkPiecewiseFunction>::New();
    volumeScalarOpacity->AddPoint(0,    0.00);
    volumeScalarOpacity->AddPoint(500,  0.15);
    volumeScalarOpacity->AddPoint(1000, 0.15);
    volumeScalarOpacity->AddPoint(1150, 0.85);


       //! The gradient opacity function is used to decrease the opacity
       //! in the "flat" regions of the volume while maintaining the opacity
       //! at the boundaries between tissue types.  The gradient is measured
       //! as the amount by which the intensity changes over unit distance.
       //! For most medical data, the unit distance is 1mm.
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
       //! The vtkVolume is a vtkProp3D (like a vtkActor) and controls the position
       //! and orientation of the volume in world coordinates.
    vtkSmartPointer<vtkVolume> volumeFixed =
            vtkSmartPointer<vtkVolume>::New();
    volumeFixed->SetMapper(volumeMapperFixed);
    volumeFixed->SetProperty(volumeProperty);

    vtkSmartPointer<vtkVolume> volumeMoving =
            vtkSmartPointer<vtkVolume>::New();
    volumeMoving->SetMapper(volumeMapperMoving);
    volumeMoving->SetProperty(volumeProperty);

    vtkSmartPointer<vtkVolume> volumeChecker =
            vtkSmartPointer<vtkVolume>::New();
    volumeChecker->SetMapper(volumeMapperChecker);
    volumeChecker->SetProperty(volumeProperty);

    vtkSmartPointer<vtkVolume> volumeDifference =
            vtkSmartPointer<vtkVolume>::New();
    volumeDifference->SetMapper(volumeMapperDifference);
    volumeDifference->SetProperty(volumeProperty);



    // Finally, add the volume to the renderer
    rendererCheckerboard->SetBackground(1,1,1);
    rendererCheckerboard->AddViewProp(volumeChecker);
    rendererCheckerboard->ResetCamera();

    rendererDifference->SetBackground(1,1,1);
    rendererDifference->AddViewProp(volumeDifference);
    rendererDifference->ResetCamera();

    rendererFixedImage ->SetBackground(1,1,1);
    rendererFixedImage ->AddViewProp(volumeChecker);
    rendererFixedImage ->ResetCamera();

    rendererMovingImage ->SetBackground(1,1,1);
    rendererMovingImage ->AddViewProp(volumeFixed);
    rendererMovingImage ->ResetCamera();


    rendererFixedImage->SetActiveCamera(rendererMovingImage->GetActiveCamera());
    rendererCheckerboard->SetActiveCamera(rendererFixedImage->GetActiveCamera());
    rendererDifference->SetActiveCamera(rendererCheckerboard->GetActiveCamera());


    // Increase the size of the render window
    renWin->SetSize(600, 600);
    renWin->Render();

      // Interact with the data.
     //iren->Initialize();
     //iren->Start();

    /**
    *
    * Visualize original
    *
    */

    int *dimensions = differenceConnector->GetOutput()->GetDimensions();

    vtkSmartPointer<vtkRenderWindow> renderWindow =
            vtkSmartPointer<vtkRenderWindow>::New();

    renderWindow->SetSize(dimensions[0] * 2, dimensions[1]);


    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
            vtkSmartPointer<vtkRenderWindowInteractor>::New();

    vtkSmartPointer<vtkRenderer> rendererLeft =
            vtkSmartPointer<vtkRenderer>::New();

    vtkSmartPointer<vtkRenderer> rendererRight =
            vtkSmartPointer<vtkRenderer>::New();

    renderWindow->AddRenderer(rendererLeft);
    renderWindow->AddRenderer(rendererRight);
    rendererLeft->SetViewport(xmins2[0],ymins2[0],xmaxs2[0],ymaxs2[0]);
    rendererRight->SetViewport(xmins2[1],ymins2[1],xmaxs2[1],ymaxs2[1]);

    interactors.push_back(renderWindowInteractor);

    vtkSmartPointer<CustomInteractor> customInteractorStyle =
            vtkSmartPointer<CustomInteractor>::New();


    renderWindowInteractor->SetRenderWindow(renderWindow);
    renderWindow->Render();
    renderWindowInteractor->SetInteractorStyle(customInteractorStyle );

    vtkSmartPointer<vtkImageMapper> imageMapperLeft = vtkSmartPointer<vtkImageMapper>::New();
    imageMapperLeft->SetInputData(checkerConnector->GetOutput());

    vtkSmartPointer<vtkImageMapper> imageMapperRight = vtkSmartPointer<vtkImageMapper>::New();
    imageMapperRight->SetInputData(differenceConnector->GetOutput());


    customInteractorStyle->SetMapper1(imageMapperLeft);
    customInteractorStyle->SetMapper2(imageMapperRight);
    customInteractorStyle->SetRenderer1(rendererLeft);
    customInteractorStyle->SetRenderer2(rendererRight);
    customInteractorStyle->SetRenderWindow(renderWindow);


    //customInteractorStyle->SetStatusMapper(sliceTextMapper);

    rendererRight->GetActiveCamera()->ParallelProjectionOn();
    rendererLeft->GetActiveCamera()->ParallelProjectionOn();

    vtkSmartPointer<vtkActor2D> imageActorLeft = vtkSmartPointer<vtkActor2D>::New();
    imageActorLeft ->SetMapper(imageMapperLeft);
    rendererLeft->AddActor2D(imageActorLeft);
    rendererLeft->SetBackground(1,1,1);

    vtkSmartPointer<vtkActor2D> imageActorRight = vtkSmartPointer<vtkActor2D>::New();
    imageActorRight ->SetMapper(imageMapperRight);
    rendererRight->AddActor2D(imageActorRight);

    rendererLeft->Render();
    rendererLeft->ResetCamera();
    rendererLeft->Render();

    rendererRight->Render();
    rendererRight->ResetCamera();
    rendererRight->Render();


    interactors[1]->Start();


    return 0;
}
