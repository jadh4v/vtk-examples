#include <vtkImageCanvasSource2D.h>
#include <vtkImageData.h>
#include <vtkImageViewer2.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

#include <array>

int main(int, char*[])
{
  vtkNew<vtkNamedColors> colors;

  std::array<double, 4> drawColor1{0, 0, 0, 0};
  auto color1 = colors->GetColor4ub("DimGray").GetData();
  std::array<double, 4> drawColor2{0, 0, 0, 0};
  auto color2 = colors->GetColor4ub("HotPink").GetData();
  for (auto i = 0; i < 4; ++i)
  {
    drawColor1[i] = color1[i];
    drawColor2[i] = color2[i];
  }
  // Set the alpha to 0 (actually alpha doesn't seem to be used)
  drawColor1[3] = 0;
  drawColor2[3] = 0;

  // Create a blank, colored image
  vtkNew<vtkImageCanvasSource2D> drawing;
  drawing->SetScalarTypeToUnsignedChar();
  drawing->SetNumberOfScalarComponents(3);
  drawing->SetExtent(0, 20, 0, 50, 0, 0);
  // Comment out or set color1 to "Black" if you want a black image
  drawing->SetDrawColor(drawColor1.data());
  drawing->FillBox(0, 20, 0, 50);

  // Draw a red circle of radius 5 centered at (9,10)
  drawing->SetDrawColor(drawColor2.data());
  drawing->DrawCircle(9, 10, 5);
  drawing->Update();

  // View the result
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  vtkNew<vtkImageViewer2> imageViewer;
  imageViewer->SetInputConnection(drawing->GetOutputPort());
  imageViewer->SetupInteractor(renderWindowInteractor);
  imageViewer->GetRenderer()->ResetCamera();
  imageViewer->GetRenderer()->SetBackground(
      colors->GetColor3d("DarkSlateGray").GetData());
  imageViewer->GetRenderWindow()->SetWindowName("DrawShapes");
  imageViewer->GetRenderWindow()->Render();
  renderWindowInteractor->Initialize();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
