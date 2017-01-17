#ifndef GAMEMAINWINDOW_H
#define GAMEMAINWINDOW_H

#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>

#include <QMainWindow>
#include <QDialog>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>


// Forward class declarations
class PyramidModel;
class Ui_GameMainWindow;
class MouseVTKInteractor;

class GameMainWindow : public QMainWindow
{
  Q_OBJECT
public:
    GameMainWindow();
    ~GameMainWindow(){}
    MouseVTKInteractor* m_mouseInteractor;
    PyramidModel *m_PyramidModel;
    vtkSmartPointer<vtkRenderer> m_renderer;
    std::vector< int > m_playersScores;
public slots:
    virtual void slotExit();
    virtual void onactionExit_2();
    void actionDefinir_la_configuration_de_la_partie();
    void actionStartGame();
    void unpublishActor(vtkSmartPointer<vtkActor> actor);
    void publishActor(vtkSmartPointer<vtkActor> actor);
    void humanPlaysThisMarble(vtkActor *actor);
    void aiPlaysThisMarble(int hashcode);
    void getInformedAboutAnActor(vtkActor *actor);

private:
    std::vector < std::vector < double > > m_coefsAI;
    void nextPlayer(bool quiet);
    Ui_GameMainWindow *ui;
    bool m_gameIsRunning;
    bool m_pyramidExist;
    int m_kindOfGame;
    int m_pyramidSize;
    int m_difficulty;
    int m_nbTotMarbles;
    int m_nbPlayers;
    int m_currentPlayer;
    int m_playedMarbles;
};

#endif
