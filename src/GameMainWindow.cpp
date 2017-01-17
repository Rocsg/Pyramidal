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
    m_playedMarbles=0;
    m_kindOfGame=0;

    //Valeurs de l'AI 0, 1, 2, 3: nb_billes, centrage_bille, altitude_bille, influence_globale
    //Valeurs de l'AI 4, 5, 6, 7: influence_locale_nb_carre_avec_une_bille, influence_locale...deux_billes, infl....trois_billes, infl...quatre billes
    m_coefsAI.push_back(std::vector < double > ({25,5,4,1,-3,12,20,12})); // Vecteur standard au feeling
    m_coefsAI.push_back(std::vector < double > ({1,5,4,5,-3,12,20,12})); // Vecteur standard au feeling
//    std::vector < double > coefsAI2 =std::vector < double > ({25,5,4,2,-3,12,20,12}); // Vecteur standard au feeling


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
    //If game is already running, or if game is over, break current game first
    if(m_gameIsRunning || (m_nbTotMarbles!=0 && (m_nbTotMarbles==m_playedMarbles))){// When game is already running, we have to break current game first.
        m_gameIsRunning=false;
        m_playersScores.clear();
        delete m_PyramidModel;
        m_renderer->RemoveAllViewProps();
    }


    //Initialize parameters
    cout<<"Début de nouvelle partie !"<<endl;
    m_playersScores=std::vector< int >(m_nbPlayers,0);
    this->ui->barrePartie->setValue(0);
    m_gameIsRunning=true;
    int score=0;
    m_playedMarbles=0;
    m_nbTotMarbles=0;
    for(int level=1;level<=m_pyramidSize;level++)m_nbTotMarbles+=(level*level);


    //initialize the model according to game parameters
    m_PyramidModel=new PyramidModel(this,m_pyramidSize,m_nbPlayers,m_nbTotMarbles,m_kindOfGame);
    m_pyramidExist=true;
    m_renderer->ResetCamera();
    m_mouseInteractor->StopState();
    m_currentPlayer=0;
    this->ui->label->setText(QString("Pyramid with ")+QString::number(m_nbTotMarbles)+
                             QString(" marbles. Your turn, player ")+QString::number(m_currentPlayer+1));


    //Run a game with AI versus AI
    if(m_kindOfGame==KOG_AI_VS_AI_GRAPHICAL){
        while(m_nbTotMarbles != m_playedMarbles){
            aiPlaysThisMarble(m_PyramidModel->nextMoveAI(0,m_currentPlayer,false,m_coefsAI[m_currentPlayer]));
            nextPlayer(false);
            if(m_nbTotMarbles != m_playedMarbles)aiPlaysThisMarble(m_PyramidModel->nextMoveAI(0,m_currentPlayer,false,m_coefsAI[m_currentPlayer]));
            nextPlayer(false);
        }
    }

    else if(m_kindOfGame==KOG_AI_VS_PLAYER){
        //If "trés difficile" selected, AI plays first. Else, it will wait for the human to play first
        if(m_difficulty==1){
            aiPlaysThisMarble(m_PyramidModel->nextMoveAI(0,m_currentPlayer,false,m_coefsAI[m_currentPlayer]));
            nextPlayer(false);
        }
    }

    else if(m_kindOfGame==KOG_AI_VS_AI_QUIET){
        while(m_nbTotMarbles != m_playedMarbles){
            aiPlaysThisMarble(m_PyramidModel->nextMoveAI(0,m_currentPlayer,false,m_coefsAI[m_currentPlayer]));
            nextPlayer(true);
            if(m_nbTotMarbles != m_playedMarbles)aiPlaysThisMarble(m_PyramidModel->nextMoveAI(0,m_currentPlayer,false,m_coefsAI[m_currentPlayer]));
            nextPlayer(true);
        }
        nextPlayer(false);
    }
}

void GameMainWindow::nextPlayer(bool quiet){
    m_currentPlayer=1-m_currentPlayer;
    if(! quiet){
        this->ui->label->setText(QString("Pyramid with ")+QString::number(m_nbTotMarbles)+
                                 QString(" marbles. Your turn, player ")+QString::number(m_currentPlayer+1));
        this->ui->scoreJ1->setText(QString("Player 1 : ")+QString::number(m_playersScores[0])+QString(" marbles"));
        this->ui->scoreJ2->setText(QString("Player 2 : ")+QString::number(m_playersScores[1])+QString(" marbles"));
        this->ui->barrePartie->setValue(100.0*m_playedMarbles/m_nbTotMarbles);
    }
    if(m_playedMarbles==m_nbTotMarbles)m_gameIsRunning=false;
    if(!m_gameIsRunning){
        cout<<"La partie est finie"<<endl;
        if(m_playersScores[0] > m_playersScores[1])this->ui->label->setText(QString("Player 1 wins !"));
        else if (m_playersScores[0] < m_playersScores[1])this->ui->label->setText(QString("Player 2 wins !"));
        else this->ui->label->setText(QString("End of match : draw !"));
        return;
    }
}




void GameMainWindow::aiPlaysThisMarble(int hashcode){
    if(!m_gameIsRunning){
        cout<<"La partie est finie"<<endl;
        return;
    }
    int score=m_PyramidModel->playThisMarble(hashcode,m_currentPlayer);
    m_playedMarbles+=score;
    if(m_playedMarbles==m_nbTotMarbles)m_gameIsRunning=false;
}




void GameMainWindow::humanPlaysThisMarble(vtkActor *actor){
    if(!m_gameIsRunning){
        cout<<"La partie est finie"<<endl;
        return;
    }

    int score=m_PyramidModel->playThisMarble(m_PyramidModel->identifyActiveMarble(actor,false),m_currentPlayer);
    m_playedMarbles+=score;
    if(score>0)
    {//prevent from clicking outside from any marble and that the AI plays as well
        nextPlayer(false);
        aiPlaysThisMarble(m_PyramidModel->nextMoveAI(0,m_currentPlayer,true,m_coefsAI[m_currentPlayer]));
        nextPlayer(false);
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
    comB->setCurrentIndex(m_difficulty);

    //Ajouter un Qchiffre pour la taille du plateau
    QLabel *labSize=new QLabel();
    labSize->setText(QString("Choose the board size :"));
    QSpinBox* spB=new QSpinBox();
    spB->setValue(m_pyramidSize);

    //Ajouter un QComboBox pour le type de partie
    QLabel *labKind=new QLabel();
    labKind->setText(QString("Choose the kind of game :"));
    QComboBox* comB2=new QComboBox();
    comB2->addItem("AI versus Player");
    comB2->addItem("AI versus AI with graphics");
    comB2->addItem("AI versus AI quiet");
    comB2->setCurrentIndex(m_kindOfGame);

    //Ajouter un QButton pour terminer les réglages
    QPushButton* valid=new QPushButton();
    valid->setText(QString("Apply changes"));

    //integrer le tout dans un vertical layout et afficher
    QGridLayout *vbox = new QGridLayout;
    vbox->addWidget(labDif,1,1,1,2);
    vbox->addWidget(comB,1,3,1,1);

    vbox->addWidget(labKind,2,1,1,2);
    vbox->addWidget(comB2,2,3,1,1);

    vbox->addWidget(labSize,3,1,1,2);
    vbox->addWidget(spB,3,3,1,1);

    vbox->addWidget(valid,4,1,2,3);
    dialog->setLayout(vbox);
    connect(valid, SIGNAL(clicked()), dialog, SLOT(accept()));
    dialog->exec();

    //Récuperation des données et action
    m_difficulty=comB->currentIndex();
    m_pyramidSize=spB->value();
    m_kindOfGame=comB2->currentIndex();
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
