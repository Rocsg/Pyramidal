#ifndef MOUSEVTKINTERACTOR_H
#define MOUSEVTKINTERACTOR_H

#include <vtkVersion.h>
#include <vtkRendererCollection.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransform.h>
#include <vtkLinearTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkObjectFactory.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkInteractorStyle.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkProperty.h>
#include "GameMainWindow.h"
//#include "PyramidModel.h"
#include <iostream>

class GameMainWindow;

class MouseVTKInteractor : public vtkInteractorStyleTrackballCamera
{
public:
//    vtkTypeMacro(MouseVTKInteractor, vtkInteractorStyleTrackballActor);
    MouseVTKInteractor(GameMainWindow * gmw);
    ~MouseVTKInteractor();
    void OnLeftButtonDown();
    void OnMiddleButtonDown();
    void OnRightButtonDown();
private:
    GameMainWindow* m_gmw;
};
//vtkStandardNewMacro(MouseVTKInteractor);


#endif // MOUSEVTKINTERACTOR_H
