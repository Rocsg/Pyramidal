#ifndef PYRAMIDMODEL_H
#define PYRAMIDMODEL_H

#include <chrono>
#include <thread>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkSmartPointer.h>
#include <vtkDepthSortPolyData.h>
#include <vtkProperty.h>

#define MARBLE_SIZE 1
#define SPACING .04
#define MAX_NB_POLYGON 30000
#define EPSILON 0.000001
class GameMainWindow;

class PyramidModel
{

public:
    PyramidModel();
    PyramidModel(GameMainWindow *gmw,int pyramidSize,int nbPlayers,int nbTotMarbles);
    ~PyramidModel();
    void setNewPlayableMarble(int level,int row, int column);
    int identifyActiveMarble(vtkActor * actor,bool verbose);
    int playThisMarble(vtkActor* actor,int player);
    vtkActor* nextMoveAI(int difficulty,int player,bool verbose,std::vector <double > aIFactors);
    void getInformedAboutAnActor(vtkActor * actor);

private:
    std::vector<std::vector<std::vector<vtkSmartPointer<vtkActor> > > > tabSphereActor;
    std::vector<std::vector<std::vector<vtkSmartPointer<vtkPolyDataMapper> > > > tabSphereMapper;
    std::vector<std::vector<std::vector<vtkSmartPointer<vtkSphereSource> > > > tabSphereSource;
    std::vector<std::vector<std::vector<int > > > gameIndicators;
    std::vector<std::vector<std::vector<int > > > AIMarkers;
    GameMainWindow* m_gmw;
    int m_currentStage;
    int m_nbPlayers;
    int m_pyramidSize;
    int m_thetRes;
    int m_phiRes;
    bool m_gameIsRunning;
    void computeAIScores(bool verbose, std::vector <double> const & aIFactors, std::vector<std::vector<std::vector < int > > > const &tabGame, std::vector < std::vector <double> > &playersScores);
    void ownedMarble(int l,int r,int c,int player);
    int howMany(int val1,int val2,int val3,int val4,int num);
    int gagneCarre(int val1,int val2, int val3, int val4,int nbPlayers);
    bool GameIsRunning();
};

#endif // PYRAMIDMODEL_H
