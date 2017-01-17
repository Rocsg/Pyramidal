#include "MouseVTKInteractor.h"
#include <vtkPropPicker.h>

using namespace std;

MouseVTKInteractor::MouseVTKInteractor(GameMainWindow * gmw) : vtkInteractorStyleTrackballCamera(),
    m_gmw(gmw){}

MouseVTKInteractor::~MouseVTKInteractor(){}


void MouseVTKInteractor::OnLeftButtonDown()
{
    int pickedOrNot=0;
    int* clickPos = this->GetInteractor()->GetEventPosition();
    vtkSmartPointer<vtkPropPicker>  picker =
         vtkSmartPointer<vtkPropPicker>::New();
    pickedOrNot=picker->Pick(clickPos[0], clickPos[1], 0, this->m_gmw->m_renderer);
    if(pickedOrNot==0){
        vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
        return;
    }
    vtkSmartPointer<vtkActor> actor = picker->GetActor();
    m_gmw->humanPlaysThisMarble(actor);
}

void MouseVTKInteractor::OnMiddleButtonDown()
{
    vtkInteractorStyleTrackballCamera::OnMiddleButtonDown();
}




void MouseVTKInteractor::OnRightButtonDown()
{
    int pickedOrNot=0;
    int* clickPos = this->GetInteractor()->GetEventPosition();
    vtkSmartPointer<vtkPropPicker>  picker =
         vtkSmartPointer<vtkPropPicker>::New();
    pickedOrNot=picker->Pick(clickPos[0], clickPos[1], 0, this->m_gmw->m_renderer);
    if(pickedOrNot==0){
        vtkInteractorStyleTrackballCamera::OnRightButtonDown();
        return;
    }
    vtkSmartPointer<vtkActor> actor = picker->GetActor();
    m_gmw->getInformedAboutAnActor(actor);
}
