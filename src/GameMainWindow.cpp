#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkCamera.h>

#include "GameMainWindow.h"
#include "PyramidModel.h"
#include "ui_GameMainWindow.h"
#include "MouseVTKInteractor.h"


/**
 * @brief GameMainWindow::GameMainWindow
 * @return Constructor called when application starts
 */
GameMainWindow::GameMainWindow()
{
    this->ui = new Ui_GameMainWindow;
    this->ui->setupUi(this);
    m_gameIsRunning=false;
    m_pyramidExist=false;
    m_difficulty=0;
    m_pyramidSize=7;
    m_nbPlayers=2;
    m_nbTotMarbles=0;


    //Make a render window and an interactor
    m_renderer=vtkSmartPointer<vtkRenderer>::New();
    vtkRenderWindow* renderWin =this->ui->qvtkWidget->GetRenderWindow();
    m_mouseInteractor = new MouseVTKInteractor(this);

    //Associate Renderer, Renderwindow and Interactor
    renderWin->AddRenderer(m_renderer);
    m_renderer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(m_mouseInteractor);
    renderWin->Render();

    // Set up action signals and slots
    connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
    connect(this->ui->actionExit_2, SIGNAL(triggered()), this, SLOT(onactionExit_2()));
    connect(this->ui->actionCommencer, SIGNAL(triggered()), this, SLOT(actionStartGame()));
    connect(this->ui->actionDefinir_la_configuration_de_la_partie, SIGNAL(triggered()), this, SLOT(actionDefinir_la_configuration_de_la_partie()));

    //Launch the game, with default parameters
    actionStartGame();
}






/**
 * @brief GameMainWindow::publishActor
 * @param actor
 * The set of functions publishActor, unpublishActor, getInformedAboutAnActor, make the link between the view and the model
 */
void GameMainWindow::publishActor(vtkSmartPointer<vtkActor> actor){
    m_renderer->AddActor(actor);
}

void GameMainWindow::unpublishActor(vtkSmartPointer<vtkActor> actor){
    m_renderer->RemoveActor(actor);
}

void GameMainWindow::getInformedAboutAnActor(vtkActor *actor){
    m_PyramidModel->getInformedAboutAnActor(actor);
}




/**
 * @brief GameMainWindow::actionStartGame
 * @return Game init at application launch, when user change the game parameters or when just starting a new game
 */
void GameMainWindow::actionStartGame()
{
    std::vector < double > coefsAI =std::vector < double > ({14,2,4,3,3,9,18,12});
    if(m_gameIsRunning || (m_nbTotMarbles==m_playedMarbles)){// When game is already running, we have to break current game first.
        m_gameIsRunning=false;
        m_playersScores.clear();
        delete m_PyramidModel;
        m_renderer->RemoveAllViewProps();
    }

    //Run a new game
    m_nbTotMarbles=0;
    for(int level=1;level<=m_pyramidSize;level++)
    {
        m_nbTotMarbles+=(level*level);
    }
    m_playedMarbles=0;
    int score=0;
    m_playersScores=std::vector< int >(m_nbPlayers,0);
    this->ui->barrePartie->setValue(0);
    m_gameIsRunning=true;
    cout<<"Début de nouvelle partie !"<<endl;

    //initialisation du plateau de jeu
    m_PyramidModel=new PyramidModel(this,m_pyramidSize,m_nbPlayers,m_nbTotMarbles);
    m_pyramidExist=true;
    m_renderer->ResetCamera();
    m_mouseInteractor->StopState();
    m_currentPlayer=0;
    this->ui->label->setText(QString("Pyramid with ")+QString::number(m_nbTotMarbles)+
                             QString(" marbles. Your turn, player ")+QString::number(m_currentPlayer+1));



    //If "trés difficile" selected, AI plays first. Else, application will wait for the human to play first
    if(m_difficulty==1){
        m_currentPlayer=1;
        thisMarblePlayed(m_PyramidModel->nextMoveAI(0,1,false,coefsAI));
    }
}



void GameMainWindow::thisMarblePlayed(vtkActor *actor){
    if(!m_gameIsRunning){
        cout<<"La partie est finie"<<endl;
        return;
    }
    std::vector < double > coefsAI =std::vector < double > ({25,5,4,2,-3,12,20,12}); // Vecteur standard au feeling
//    std::vector < double > coefsAI =std::vector < double > ({1,0,0,0,0,0,0,0}); //Test du facteur billes --> ok
//    std::vector < double > coefsAI =std::vector < double > ({0,1,0,0,0,0,0,0}); //Test center --> ok
//    std::vector < double > coefsAI =std::vector < double > ({0,0,1,0,0,0,0,0}); //Test altitude --> ok
//    std::vector < double > coefsAI =std::vector < double > ({0,0,0,1,0,0,0,0}); //Test qui ferait de l'occupation de l'espace --> parfait
//    std::vector < double > coefsAI =std::vector < double > ({0,0,0,0,1,0,0,0}); //Test qui ferait des 1 --> ok
//    std::vector < double > coefsAI =std::vector < double > ({0,0,0,0,0,1,0,0}); //Test qui ferait des 2 ? --> ok
//    std::vector < double > coefsAI =std::vector < double > ({0,0,0,0,0,0,1,0}); //Test qui ferait des 3 --> ok
//    std::vector < double > coefsAI =std::vector < double > ({0,0,0,0,0,0,0,1}); //Test qui ferait des 4 --> ok

    int score=m_PyramidModel->playThisMarble(actor,m_currentPlayer);
    m_playedMarbles+=score;
    this->ui->scoreJ1->setText(QString("Player 1 : ")+QString::number(m_playersScores[0])+QString(" marbles"));
    this->ui->scoreJ2->setText(QString("Player 2 : ")+QString::number(m_playersScores[1])+QString(" marbles"));
    this->ui->barrePartie->setValue(100.0*m_playedMarbles/m_nbTotMarbles);
    if(m_playedMarbles==m_nbTotMarbles)m_gameIsRunning=false;


    if(!m_gameIsRunning){
        cout<<"La partie est finie"<<endl;
        if(m_playersScores[0]>m_playersScores[1])this->ui->label->setText(QString("Player 1 wins !"));
        else this->ui->label->setText(QString("Player 2 wins !"));
        return;
    }

    if(score>0){
        m_currentPlayer++;
        if(m_currentPlayer==m_nbPlayers)m_currentPlayer=0;
        this->ui->label->setText(QString("Pyramid with ")+QString::number(m_nbTotMarbles)+
                                 QString(" marbles. Your turn, player ")+QString::number(m_currentPlayer+1));
        if(m_currentPlayer==1)thisMarblePlayed(m_PyramidModel->nextMoveAI(0,1,true,coefsAI));
    }
}

void GameMainWindow::actionDefinir_la_configuration_de_la_partie()
{
    QDialog* dialog=new QDialog();
    dialog->setWindowIconText(QString("Configuration"));

    //Ajouter un QCombobox pour la difficulté
    QLabel *labDif=new QLabel();
    labDif->setText(QString("Choose difficulty level :"));
    QComboBox* comB=new QComboBox();
    comB->addItem("Difficult (You play first)");
    comB->addItem("Very difficult (AI plays first)");
    comB->setCurrentIndex(0);

    //Ajouter un Qchiffre pour la taille du plateau
    QLabel *labSize=new QLabel();
    labSize->setText(QString("Choose the board size :"));
    QSpinBox* spB=new QSpinBox();
    spB->setValue(7);

    //Ajouter un QButton pour terminer les réglages
    QPushButton* valid=new QPushButton();
    valid->setText(QString("Apply changes"));

    //integrer le tout dans un vertical layout et afficher
    QGridLayout *vbox = new QGridLayout;
    vbox->addWidget(labDif,1,1,1,2);
    vbox->addWidget(comB,1,3,1,1);
    vbox->addWidget(labSize,2,1,1,2);
    vbox->addWidget(spB,2,3,1,1);
    vbox->addWidget(valid,3,1,2,3);
    dialog->setLayout(vbox);
    connect(valid, SIGNAL(clicked()), dialog, SLOT(accept()));
    dialog->exec();

    //Récuperation des données et action
    m_difficulty=comB->currentIndex();
    m_pyramidSize=spB->value();
    delete comB;
    delete spB;
    delete valid;
    delete dialog;
    actionStartGame();
}

void GameMainWindow::onactionExit_2()
{
  qApp->exit();
}


void GameMainWindow::slotExit()
{
  qApp->exit();
}
