#!/usr/bin/env python

import os.path

# noinspection PyUnresolvedReferences
import vtkmodules.vtkInteractionStyle
# noinspection PyUnresolvedReferences
import vtkmodules.vtkRenderingOpenGL2
from vtkmodules.vtkCommonColor import vtkNamedColors
from vtkmodules.vtkCommonDataModel import vtkPolyData
from vtkmodules.vtkFiltersCore import (
    vtkDecimatePro,
    vtkTriangleFilter
)
from vtkmodules.vtkFiltersSources import vtkSphereSource
from vtkmodules.vtkIOGeometry import (
    vtkBYUReader,
    vtkOBJReader,
    vtkSTLReader
)
from vtkmodules.vtkIOPLY import vtkPLYReader
from vtkmodules.vtkIOXML import vtkXMLPolyDataReader
from vtkmodules.vtkRenderingCore import (
    vtkActor,
    vtkCamera,
    vtkPolyDataMapper,
    vtkProperty,
    vtkRenderWindow,
    vtkRenderWindowInteractor,
    vtkRenderer
)


def get_program_parameters():
    import argparse
    description = 'Decimate polydata.'
    epilogue = '''
    This is an example using vtkDecimatePro to decimate input polydata, if provided, or a sphere otherwise.
    '''
    parser = argparse.ArgumentParser(description=description, epilog=epilogue)
    parser.add_argument('filename', nargs='?', default=None, help='Optional input filename e.g Torso.vtp.')
    parser.add_argument('reduction', nargs='?', type=float, default=.9,
                        help='Sets the decimation target reduction, (default is 0.9).')
    args = parser.parse_args()
    return args.filename, args.reduction


def main():
    filePath, reduction = get_program_parameters()

    # Define colors
    colors = vtkNamedColors()
    backFaceColor = colors.GetColor3d('Gold')
    inputActorColor = colors.GetColor3d('NavajoWhite')
    decimatedActorColor = colors.GetColor3d('NavajoWhite')
    # colors.SetColor('leftBkg', [0.6, 0.5, 0.4, 1.0])
    # colors.SetColor('rightBkg', [0.4, 0.5, 0.6, 1.0])

    if filePath and os.path.isfile(filePath):
        readerPD = ReadPolyData(filePath)
        if not readerPD:
            inputPolyData = GetSpherePD()
        else:
            triangles = vtkTriangleFilter()
            triangles.SetInputData(readerPD)
            triangles.Update()
            inputPolyData = triangles.GetOutput()
    else:
        inputPolyData = GetSpherePD()

    print('Before decimation')
    print(f'There are {inputPolyData.GetNumberOfPoints()} points.')
    print(f'There are {inputPolyData.GetNumberOfPolys()} polygons.')

    decimate = vtkDecimatePro()
    decimate.SetInputData(inputPolyData)
    decimate.SetTargetReduction(reduction)
    decimate.PreserveTopologyOn()
    decimate.Update()

    decimated = vtkPolyData()
    decimated.ShallowCopy(decimate.GetOutput())

    print('After decimation')
    print(f'There are {decimated.GetNumberOfPoints()} points.')
    print(f'There are {decimated.GetNumberOfPolys()} polygons.')
    print(
        f'Reduction: {(inputPolyData.GetNumberOfPolys() - decimated.GetNumberOfPolys()) / inputPolyData.GetNumberOfPolys()}')

    inputMapper = vtkPolyDataMapper()
    inputMapper.SetInputData(inputPolyData)

    backFace = vtkProperty()
    backFace.SetColor(backFaceColor)

    inputActor = vtkActor()
    inputActor.SetMapper(inputMapper)
    inputActor.GetProperty().SetInterpolationToFlat()
    inputActor.GetProperty().SetColor(inputActorColor)
    inputActor.SetBackfaceProperty(backFace)

    decimatedMapper = vtkPolyDataMapper()
    decimatedMapper.SetInputData(decimated)

    decimatedActor = vtkActor()
    decimatedActor.SetMapper(decimatedMapper)
    decimatedActor.GetProperty().SetColor(decimatedActorColor)
    decimatedActor.GetProperty().SetInterpolationToFlat()
    decimatedActor.SetBackfaceProperty(backFace)

    # There will be one render window
    renderWindow = vtkRenderWindow()
    renderWindow.SetSize(600, 300)
    renderWindow.SetWindowName('Decimation');

    # And one interactor
    interactor = vtkRenderWindowInteractor()
    interactor.SetRenderWindow(renderWindow)

    # Define viewport ranges
    # (xmin, ymin, xmax, ymax)
    leftViewport = [0.0, 0.0, 0.5, 1.0]
    rightViewport = [0.5, 0.0, 1.0, 1.0]

    # Setup both renderers
    leftRenderer = vtkRenderer()
    renderWindow.AddRenderer(leftRenderer)
    leftRenderer.SetViewport(leftViewport)
    # leftRenderer.SetBackground((colors.GetColor3d('leftBkg')))
    leftRenderer.SetBackground((colors.GetColor3d('Peru')))

    rightRenderer = vtkRenderer()
    renderWindow.AddRenderer(rightRenderer)
    rightRenderer.SetViewport(rightViewport)
    # rightRenderer.SetBackground((colors.GetColor3d('rightBkg')))
    rightRenderer.SetBackground((colors.GetColor3d('CornflowerBlue')))

    # Add the sphere to the left and the cube to the right
    leftRenderer.AddActor(inputActor)
    rightRenderer.AddActor(decimatedActor)

    # Shared camera
    # Shared camera looking down the -y axis
    camera = vtkCamera()
    camera.SetPosition(0, -1, 0)
    camera.SetFocalPoint(0, 0, 0)
    camera.SetViewUp(0, 0, 1)
    camera.Elevation(30)
    camera.Azimuth(30)

    leftRenderer.SetActiveCamera(camera)
    rightRenderer.SetActiveCamera(camera)

    leftRenderer.ResetCamera()
    leftRenderer.ResetCameraClippingRange()

    renderWindow.Render()
    renderWindow.SetWindowName('Decimation')

    interactor.Start()


def ReadPolyData(file_name):
    import os
    path, extension = os.path.splitext(file_name)
    extension = extension.lower()
    if extension == '.ply':
        reader = vtkPLYReader()
        reader.SetFileName(file_name)
        reader.Update()
        poly_data = reader.GetOutput()
    elif extension == '.vtp':
        reader = vtkXMLPolyDataReader()
        reader.SetFileName(file_name)
        reader.Update()
        poly_data = reader.GetOutput()
    elif extension == '.obj':
        reader = vtkOBJReader()
        reader.SetFileName(file_name)
        reader.Update()
        poly_data = reader.GetOutput()
    elif extension == '.stl':
        reader = vtkSTLReader()
        reader.SetFileName(file_name)
        reader.Update()
        poly_data = reader.GetOutput()
    elif extension == '.vtk':
        reader = vtkpoly_dataReader()
        reader.SetFileName(file_name)
        reader.Update()
        poly_data = reader.GetOutput()
    elif extension == '.g':
        reader = vtkBYUReader()
        reader.SetGeometryFileName(file_name)
        reader.Update()
        poly_data = reader.GetOutput()
    else:
        # Return a None if the extension is unknown.
        poly_data = None
    return poly_data


def GetSpherePD():
    '''
    :return: The PolyData representation of a sphere.
    '''
    source = vtkSphereSource()
    source.SetThetaResolution(30)
    source.SetPhiResolution(15)
    source.Update()
    return source.GetOutput()


if __name__ == '__main__':
    main()
