// test baking shadow maps

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCameraPass.h>
#include <vtkCellArray.h>
#include <vtkCubeSource.h>
#include <vtkLight.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOpenGLTexture.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderPassCollection.h>
#include <vtkRenderStepsPass.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSequencePass.h>
#include <vtkShadowMapBakerPass.h>
#include <vtkShadowMapPass.h>
#include <vtkSmartPointer.h>

#include <vtkBYUReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkSphereSource.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkMetaImageReader.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkSSAOPass.h>
#include <vtkImageData.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolume.h>

#include <vtksys/SystemTools.hxx>

#include <array>

namespace {
vtkSmartPointer<vtkPolyData> ReadPolyData(const char* fileName);
vtkSmartPointer<vtkImageData> ReadImageData(const char* fileName);
}

//----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  // Read the polyData
  auto imageData = ReadImageData(argc > 2 ? argv[2] : "");

  vtkNew<vtkNamedColors> colors;
  colors->SetColor("HighNoonSun", 1.0, 1.0, .9843, 1.0); // Color temp. 5400k
  colors->SetColor("100W Tungsten", 1.0, .8392, .6667,
                   1.0); // Color temp. 2850k

  vtkNew<vtkRenderer> renderer;
  renderer->SetBackground(colors->GetColor3d("Silver").GetData());

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetSize(640, 480);
  renderWindow->AddRenderer(renderer);

  vtkNew<vtkInteractorStyleTrackballCamera> style;
  vtkNew<vtkRenderWindowInteractor> interactor;
  interactor->SetInteractorStyle(style);
  interactor->SetRenderWindow(renderWindow);

  vtkNew<vtkOpenGLGPUVolumeRayCastMapper> mapper;
  mapper->SetInputData(imageData);

  vtkNew<vtkPiecewiseFunction> compositeOpacity;
  compositeOpacity->AddPoint(0.0, 0.0);
  compositeOpacity->AddPoint(80.0, 1.0);
  compositeOpacity->AddPoint(80.1, 0.0);
  compositeOpacity->AddPoint(255.0, 0.0);

  vtkNew<vtkColorTransferFunction> color;
  color->AddRGBPoint(0.0, 0.0, 0.0, 1.0);
  color->AddRGBPoint(40.0, 1.0, 0.0, 0.0);
  color->AddRGBPoint(255.0, 1.0, 1.0, 1.0);

  vtkNew<vtkVolumeProperty> volumeProperty;
  volumeProperty->ShadeOff();
  volumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);
  volumeProperty->SetScalarOpacity(compositeOpacity); // composite first.
  volumeProperty->SetColor(color);

  vtkNew<vtkVolume> actor;
  actor->SetMapper(mapper);
  actor->SetProperty(volumeProperty);

  vtkNew<vtkRenderStepsPass> basicPasses;
  vtkNew<vtkSSAOPass> ssao;
  ssao->SetRadius(0.05);
  ssao->SetKernelSize(128);
  ssao->SetDelegatePass(basicPasses);

  vtkOpenGLRenderer* glrenderer = vtkOpenGLRenderer::SafeDownCast(renderer);

  // this step causes an error, comment this out for error-free execution.
  glrenderer->SetPass(ssao);

  renderer->AddActor(actor);
  renderer->ResetCamera();
  renderWindow->Render();
  renderWindow->SetWindowName("Shadows");

  interactor->Start();

  return EXIT_SUCCESS;
}

namespace {

vtkSmartPointer<vtkPolyData> ReadPolyData(const char* fileName)
{
  vtkSmartPointer<vtkPolyData> polyData;
  std::string extension =
      vtksys::SystemTools::GetFilenameLastExtension(std::string(fileName));
  if (extension == ".ply")
  {
    vtkNew<vtkPLYReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".vtp")
  {
    vtkNew<vtkXMLPolyDataReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".obj")
  {
    vtkNew<vtkOBJReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".stl")
  {
    vtkNew<vtkSTLReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".vtk")
  {
    vtkNew<vtkPolyDataReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".g")
  {
    vtkNew<vtkBYUReader> reader;
    reader->SetGeometryFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else
  {
    vtkNew<vtkSphereSource> source;
    source->SetThetaResolution(100);
    source->SetPhiResolution(100);
    source->Update();
    polyData = source->GetOutput();
  }
  return polyData;
}

vtkSmartPointer<vtkImageData> ReadImageData(const char* fileName)
{
  vtkSmartPointer<vtkImageData> image;
  vtkNew<vtkMetaImageReader> reader;
  if (reader->CanReadFile(fileName))
  {
    reader->SetFileName(fileName);
    reader->Update();
    image = reader->GetOutput();
  }
  return image;
}

} // namespace
