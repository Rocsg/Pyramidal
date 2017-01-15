#include "PyramidModel.h"
#include "GameMainWindow.h"
#include "MouseVTKInteractor.h"

double min(double a1,double a2){return (a1<a2 ? a1 : a2);}
double max(double a1,double a2){return (a1<a2 ? a2 : a1);}

PyramidModel::PyramidModel()
{
    cout<<"Initialisation du PyramidModel par constructeur par défaut"<<endl;
    m_gameIsRunning=true;
    cout<<"   --> ok"<<endl<<endl;
}


PyramidModel::PyramidModel(GameMainWindow *gmw,int pyramidSize,int nbPlayers,int nbTotMarbles):
    gameIndicators(pyramidSize,std::vector<std::vector<int > > (pyramidSize, std::vector<int > (pyramidSize,-3))),
    AIMarkers(pyramidSize,std::vector<std::vector<int > > (pyramidSize, std::vector<int > (pyramidSize,-3))),
    tabSphereActor(pyramidSize,std::vector<std::vector<vtkSmartPointer<vtkActor> > > (pyramidSize, std::vector<vtkSmartPointer<vtkActor> > (pyramidSize,NULL))),
    tabSphereSource(pyramidSize,std::vector<std::vector<vtkSmartPointer<vtkSphereSource> > > (pyramidSize, std::vector<vtkSmartPointer<vtkSphereSource> > (pyramidSize,NULL))),
    tabSphereMapper(pyramidSize,std::vector<std::vector<vtkSmartPointer<vtkPolyDataMapper> > > (pyramidSize, std::vector<vtkSmartPointer<vtkPolyDataMapper> > (pyramidSize,NULL))),
    m_nbPlayers(nbPlayers),
    m_pyramidSize(pyramidSize),
    m_gmw(gmw)
{

    //Calcul de la resolution graphique des billes
    int polygonPerMarble=MAX_NB_POLYGON/nbTotMarbles;
    int val=sqrt(polygonPerMarble/2);
    m_thetRes=2*val;
    m_phiRes=val;
    m_thetRes= (6 <= m_thetRes) ? m_thetRes : 6;
    m_phiRes= (4 <= m_phiRes) ? m_phiRes : 4;
    int totPolygon=m_phiRes *m_thetRes* nbTotMarbles;
    if(totPolygon>MAX_NB_POLYGON){
        cout<<"Warning : end of game will see "<<totPolygon<<" polygons displayed. There exists a risk of graphical overflow"<<endl;
    }
    cout<<"Résolution choisie pour chaque bille : selon le diametre ="<<m_thetRes<<" et selon un meridien ="<<m_phiRes<<endl;


    //initialiser la zone de utilisable dans le jeu et la zone jouable en début de partie : le bas du plateau
    for(int level=0;level<pyramidSize;level++)
    {
        for(int i=0;i<pyramidSize-level;i++)
        {
            for(int j=0;j<pyramidSize-level;j++)
            {
                gameIndicators[level][i][j]=-2;
                if(level==0)setNewPlayableMarble(0,i,j);
            }
        }
    }

   m_gameIsRunning=true;
   m_currentStage=0;
}


PyramidModel::~PyramidModel()
{
    //initialiser la zone de jeu du tableau de la partie
    int i=0,j=0,k=0,indI,indJ,indK;
    while(tabSphereSource.size()>0){
        indI=tabSphereSource.size()-1;

        while(tabSphereSource[indI].size()>0){
            indJ=tabSphereSource[indI].size()-1;

            while(tabSphereSource[indI][indJ].size()>0){
                gameIndicators[indI][indJ].pop_back();
                AIMarkers[indI][indJ].pop_back();
                tabSphereActor[indI][indJ].pop_back();
                tabSphereMapper[indI][indJ].pop_back();
                tabSphereSource[indI][indJ].pop_back();
            }
            gameIndicators[indI].pop_back();
            AIMarkers[indI].pop_back();
            tabSphereActor[indI].pop_back();
            tabSphereMapper[indI].pop_back();
            tabSphereSource[indI].pop_back();
        }
        gameIndicators.pop_back();
        AIMarkers.pop_back();
        tabSphereActor.pop_back();
        tabSphereMapper.pop_back();
        tabSphereSource.pop_back();
    }
    cout<<"Suppression finie"<<endl;
}


void PyramidModel::setNewPlayableMarble(int level,int row, int column){
    gameIndicators[level][row][column]=0;
    if(tabSphereSource[level][row][column]!=NULL){
        cout<<"Tentative d'ajout d'une bille jouable déja existante à "<<level<<","<<row<<","<<column<<endl;
        return;
    }
    tabSphereSource[level][row][column]=vtkSmartPointer<vtkSphereSource>::New();
    tabSphereSource[level][row][column]->SetRadius(MARBLE_SIZE);
    tabSphereSource[level][row][column]->SetThetaResolution(m_thetRes);//24 min : 8
    tabSphereSource[level][row][column]->SetPhiResolution(m_phiRes);//12 min : 4
    tabSphereSource[level][row][column]->SetCenter(MARBLE_SIZE*2*(1+SPACING)*(row+level/2.0),MARBLE_SIZE*2*(1+SPACING)*(column+level/2.0) ,MARBLE_SIZE*(level)*sqrt(3)/1.25 );
    tabSphereSource[level][row][column]->Update();

    tabSphereMapper[level][row][column]= vtkSmartPointer<vtkPolyDataMapper>::New();
    tabSphereMapper[level][row][column]->SetInputConnection(tabSphereSource[level][row][column]->GetOutputPort());

    tabSphereActor[level][row][column]=vtkSmartPointer<vtkActor>::New();
    (tabSphereActor[level][row][column])->SetMapper(tabSphereMapper[level][row][column]);
    (tabSphereActor[level][row][column])->GetProperty()->SetColor(0.6+0.4*(level+1)/(1.0*m_pyramidSize), 0.6+0.4*(level+1)/(1.0*m_pyramidSize), 0.6+0.4*(level+1)/(1.0*m_pyramidSize)); //(R,G,B)
    (tabSphereActor[level][row][column])->GetProperty()->SetOpacity(0.20); //(R,G,B)
    (tabSphereActor[level][row][column])->GetProperty()->SetInterpolationToPhong(); //(R,G,B)
    m_gmw->publishActor(tabSphereActor[level][row][column]);
    if(level>m_currentStage){
        m_currentStage=level;
        m_gmw->m_renderer->ResetCamera();
    }
}


void PyramidModel::getInformedAboutAnActor(vtkActor *actor){
    if (actor==NULL){
        cout<<"Pas de bille selectionnée. Inspection terminée"<<endl;
        return;
    }
    for(int level=0;level<m_pyramidSize;level++)
    {
        for(int row=0;row<m_pyramidSize-level;row++)
        {
            for(int col=0;col<m_pyramidSize-level;col++)
            {
                if((gameIndicators[level][row][col])>=0 &&  actor==tabSphereActor[level][row][col])
                {
                    cout<<"Coordonnées concernées : "<<level<<","<<row<<","<<col<<"  ---  ";
                    cout<<"Etat de la bille : "<<gameIndicators[level][row][col]<<endl;
                }
            }
        }
    }
    return;
}


int PyramidModel::identifyActiveMarble(vtkActor* actor,bool verbose){
    if (actor==NULL)return -1;
    int ret;
    for(int level=0;level<m_pyramidSize;level++)
    {
        for(int row=0;row<m_pyramidSize-level;row++)
        {
            for(int col=0;col<m_pyramidSize-level;col++)
            {
                if((gameIndicators[level][row][col])>=0 &&  actor==tabSphereActor[level][row][col])
                {
                    if(verbose)cout<<"Je l'ai ! La bille est : "<<level<<","<<row<<","<<col<<" d'opacité : "<<(tabSphereActor[level][row][col])->GetProperty()->GetOpacity()<<endl;
                    ret=m_pyramidSize*m_pyramidSize*level+m_pyramidSize*row+col;
                }
            }
        }
    }
    return ret;
}










/**
 * @brief PyramidModel::computeAIScores
 * @param verbose
 * @param aIFactors
 * @param tabGame
 * @return a 2-double std::vector giving players score in the current model, given in tabGame
 */
void PyramidModel::computeAIScores(bool verbose, std::vector <double> const & aIFactors,
                                   std::vector<std::vector<std::vector < int > > > const &tabGame, std::vector < std::vector <double> > &playersScore){

    if(aIFactors.size()<8){
        cout<<"Erreur critique : moins de 8 facteurs de l'AI"<<endl;
        exit;
    }


    /////// STEP 1/3 Compute marbles score = Nb of owned marbles
    for(int l=0;l<m_pyramidSize-1;l++){
        for(int r=0;r<m_pyramidSize-l;r++){
            for(int c=0;c<m_pyramidSize-l;c++){
                if(tabGame[l][r][c]>0)(playersScore[tabGame[l][r][c]-1][0])++;
            }
        }
    }



    ///////  STEP 2/3 Compute shape factor = AIFACTORSHAPE x entries : Nb of marble under sight in the base level
    bool graphicVerbose=false;
    int nbAdd;
    int pass;
    int l=0,r,c,scTmp1,scTmp2,tmp;
    double d;
    std::vector < std::vector < std::vector <double> > > distances = std::vector < std::vector <std::vector <double> > >
                     (2, std::vector <std::vector <double> > (m_pyramidSize,std::vector <double>(m_pyramidSize,1000)));
    std::vector< std::vector < int > > add;// Visited marbles at the last step
    std::vector< std::vector < int > > next;// Marbles to visit a the next step
    std::vector< int > mask=std::vector < int > (8,0);// This mask will be used for checking possibility of visiting the 8 neighbours
    std::vector< int > maskR=std::vector < int > ({-1,0,1,-1,1,-1,0,1}) ;// These tabs refers to the relatives coordinates of the neighbours, along r axis
    std::vector< int > maskC=std::vector < int > ({-1,-1,-1,0,0,1,1,1}) ;// And along c axis
    std::vector< double > maskD=std::vector < double > ({1.5,1,1.5,1,1,1.5,1,1.5}) ;// And here is the distance function used, see ASCII draw below
    // ---------> (r axe)
    // | 0 1 2
    // | 3 W 4
    // | 5 6 7
    // V (c axe)


    //Djikstra distance function for the two players
    for(int play=0;play<2;play++){
        add.clear();
        next.clear();
        nbAdd=0;
        //First pass preparation
        for(int r=0;r<m_pyramidSize;r++){
            for(int c=0;c<m_pyramidSize;c++){
                if(tabGame[0][r][c]==play+1){
                    next.push_back(std::vector<int> (2,0));
                    next[nbAdd][0]=r;
                    next[nbAdd][1]=c;
                    distances[play][r][c]=0;
                    nbAdd++;
                }
            }
        }

        pass=0;

        // Until no more move
        while(nbAdd>0){

            //Visited last time <-- to visit next time
            add.clear();
            tmp=next.size();
            for(int i =0;i<tmp;i++)add.push_back(next[i]);
            next.clear();
            nbAdd=0;

            //For each last time visited
            for(int ad=0;ad<add.size() ; ad++){
                r=add[ad][0];
                c=add[ad][1];
                d=distances[play][r][c];

                //init neighbour visit mask
                for(int vis=0;vis<8 ;vis++)mask[vis]=1;
                if(r == m_pyramidSize-1){//There are restrictions about going "right"
                    mask[2]=0;mask[4]=0; mask[7]=0;
                }
                if(c == m_pyramidSize-1){//There are restrictions about going "down"
                    mask[5]=0; mask[6]=0; mask[7]=0;
                }
                if(c == 0){//There are restrictions about going "up"
                    mask[0]=0; mask[1]=0;mask[2]=0;
                }
                if(r == 0){//There are restrictions about going "left"
                    mask[0]=0; mask[3]=0; mask[5]=0;
                }

                //visit available neighbours
                //if the visit is possible, visit, add to the "next" list and actualize its distance
                for(int i=0;i<8;i++){
                    if(mask[i]==1){
                        if(tabGame[0][r+maskR[i]][c+maskC[i]]==0 && distances[play][r+maskR[i]][c+maskC[i]]>(d+maskD[i])){
                            distances[play][r+maskR[i]][c+maskC[i]]=d+maskD[i];
                            next.push_back(std::vector<int> (2,0));
                            next[nbAdd][0]=r+maskR[i];
                            next[nbAdd][1]=c+maskC[i];
                            nbAdd++;
                        }
                    }
                }
            }
            pass++;
        }
    }

    //  Score the influence zones
    for(int r=0;r<m_pyramidSize;r++){
        for(int c=0;c<m_pyramidSize;c++){
            if(distances[0][r][c]<(distances[1][r][c]-EPSILON) ){
                (playersScore[0][3])++;
            }
            else if(distances[1][r][c]<(distances[0][r][c]-EPSILON) ){
                (playersScore[1][3])++;
            }
        }
    }


    ///////  STEP 3/3 Compute local influence score = dependant to the number and kind of local situations where present
    for(int l=0;l<m_pyramidSize-1;l++){
        for(int r=0;r<m_pyramidSize-l-1;r++){
            for(int c=0;c<m_pyramidSize-l-1;c++){
               scTmp1=howMany(tabGame[l][r][c],tabGame[l][r+1][c+1],tabGame[l][r][c+1],tabGame[l][r+1][c],1);
               scTmp2=howMany(tabGame[l][r][c],tabGame[l][r+1][c+1],tabGame[l][r][c+1],tabGame[l][r+1][c],2);
               if(scTmp1>0){
                   playersScore[0][3+scTmp1]++;
               }
               if(scTmp2>0){
                   playersScore[1][3+scTmp2]++;
               }
            }
        }
    }
    distances.clear();
    add.clear();
    next.clear();
    maskR.clear();
    maskC.clear();
    maskD.clear();
}





/**
 * @brief PyramidModel::nextMoveAI
 * @param difficulty
 * @param currentPlayer
 * @param verbose
 * @param aIFactors
 * @return The vtkActor* representing the selected marble that the AI wants to play
 */
vtkActor* PyramidModel::nextMoveAI(int difficulty,int currentPlayer,bool verbose,std::vector <double> aIFactors){
    if(aIFactors.size()<8){
        cout<<"Erreur critique : moins de 8 facteurs de l'AI"<<endl;
        exit;
    }
    if(verbose)cout<<endl<<"AI : prochain coup en cours de calcul"<<endl;
    int adverse=1-currentPlayer;

    //Get current AI parameters
    double AI_FACTOR_BILLES=aIFactors[0];//14
    double AI_FACTOR_CENTER=aIFactors[1];//2
    double AI_FACTOR_ALTITUDE=aIFactors[2];//4
    double AI_FACTOR_SHAPE=aIFactors[3];//3
    double AI_FACTOR_OWN_1=aIFactors[4];//3
    double AI_FACTOR_OWN_2=aIFactors[5];//9
    double AI_FACTOR_OWN_3=aIFactors[6];//18
    double AI_FACTOR_OWN_4=aIFactors[7];//12

    std::vector< int > mask=std::vector<int>(4,1);//Displacement mask for possible appearing marbles on the upper level of the current marble (W)
    std::vector< int > maskR=std::vector< int >({-1,0,-1,0});//Here are the "r axe" relatives displacements from current to upper
    std::vector< int > maskC=std::vector< int >({-1,-1,0,0});//Here are the "c axe". See mask scheme below
    //-----------> (r axe)
    //| 0   1
    //|   W
    //| 2   3
    //|
    //V (c axe)


    //Inventorier les coups possibles
    std::vector< std::vector <int> > moves;
    int possMoves=0;
    for(int l=0;l<m_pyramidSize;l++){
        for(int r=0;r<m_pyramidSize-l;r++){
            for(int c=0;c<m_pyramidSize-l;c++){
                if(gameIndicators[l][r][c]==0){
                    moves.push_back(std::vector<int>(3,0)); //l,r,c
                    moves[possMoves][0]=l;
                    moves[possMoves][1]=r;
                    moves[possMoves][2]=c;
                    possMoves++;
                }
            }
        }
    }
    std::vector< std::vector< std::vector< double > >  > movesScore=std::vector<std::vector< std::vector< double > > >
            (possMoves,std::vector< std::vector< double > > (2, std::vector< double >(8,0)));
    std::vector< double > scoreFinalMove=std::vector< double > (possMoves,0);
    std::vector< std::vector<double> >playersScores=std::vector< std::vector<double> > (2,std::vector<double>(8,0));
    if(verbose)cout<<"Nombre de coups possibles : "<<possMoves<<endl;



    //Essayer chaque coup possible
    for(int mov=0;mov<possMoves;mov++){

        //Recopier la grille actuelle
        for(int l=0;l<m_pyramidSize;l++){
            for(int r=0;r<m_pyramidSize-l;r++){
                for(int c=0;c<m_pyramidSize-l;c++){
                    AIMarkers[l][r][c]=gameIndicators[l][r][c];
                }
            }
        }

        //jouer le coup
        int nbAdd=1;
        int nbAddTot=1;
        int pass=0;
        int l,r,c,val1,val2,val3,val4,gag,tmp,scTmp;
        double scTmpD;
        std::vector< std::vector < int > > add;
        std::vector< std::vector < int > > next;
        next.push_back(std::vector<int> (3,0));
        next[0][0]=moves[mov][0];
        next[0][1]=moves[mov][1];
        next[0][2]=moves[mov][2];
        AIMarkers[moves[mov][0]][moves[mov][1]][moves[mov][2]]=currentPlayer+1;

        //Parcours en largeur récursif des ajouts de billes
        while(nbAdd>0){
            nbAdd=0;

            //Les next d'avant deviennent les add d'aujourd'hui
            add.clear();
            tmp=next.size();
            for(int i =0;i<tmp;i++)add.push_back(next[i]);
            next.clear();
            //for each marble added last time, actualization of new playing possibilities and new owned marbles
            for(int ad=0;ad<add.size();ad++)
            {
                l=add[ad][0];
                r=add[ad][1];
                c=add[ad][2];
                for(int i=0;i<4;i++)mask[i]=1;
                if(r==m_pyramidSize-l-1){
                    mask[1]=0;mask[3]=0;
                }
                if(c==m_pyramidSize-l-1){
                    mask[2]=0;mask[3]=0;
                }
                if(r==0){
                    mask[0]=0;mask[2]=0;
                }
                if(c==0){
                    mask[0]=0;mask[1]=0;
                }



                //check for possible child marbles (upper level) for the 4 configurations of the mask around this marble
                for(int i=0;i<4;i++){
                    if(mask[i]==1){
                        if(AIMarkers[l][r+maskR[i]][c+maskC[i]]>0 && AIMarkers[l][r+1+maskR[i]][c+maskC[i]+1]>0 &&
                                AIMarkers[l][r+maskR[i]][c+maskC[i]+1]>0 && AIMarkers[l][r+maskR[i]+1][c+maskC[i]]>0 && AIMarkers[l+1][r+maskR[i]][c+maskC[i]]<=0){
                            val1=AIMarkers[l][r+maskR[i]][c+maskC[i]];
                            val2=AIMarkers[l][r+maskR[i]+1][c+maskC[i]];
                            val3=AIMarkers[l][r+maskR[i]][c+maskC[i]+1];
                            val4=AIMarkers[l][r+maskR[i]+1][c+maskC[i]+1];
                            gag=gagneCarre(val1,val2,val3,val4,m_nbPlayers);
                            if(gag==0){AIMarkers[l+1][r+maskR[i]][c+maskC[i]]=0;}
                            else{
                                AIMarkers[l+1][r+maskR[i]][c+maskC[i]]=gag;
                                next.push_back(std::vector<int>(3,0));
                                next[nbAdd][0]=l+1;
                                next[nbAdd][1]=r+maskR[i];
                                next[nbAdd][2]=c+maskC[i];
                                nbAdd++;
                            }
                        }
                    }
                }
            }
            pass++;
         }

        //Calculer le score lié à la configuration finale du plateau après le coup
        for(int play=0;play<2;play++)for(int parm=0;parm<8;parm++)playersScores[play][parm]=0;
        computeAIScores(true, aIFactors,AIMarkers,playersScores);
        for(int play=0;play<2;play++)for(int parm=0;parm<8;parm++)movesScore[mov][play][parm]=playersScores[play][parm];


        //Calculer le score lié au placement de la bille elle même : FACTOR_CENTER et FACTOR_ALTITUDE
        //Factor center
        scTmpD=sqrt(  (moves[mov][1]-(m_pyramidSize-moves[mov][0])/2)*(moves[mov][1]-(m_pyramidSize-moves[mov][0])/2)
                     +(moves[mov][2]-(m_pyramidSize-moves[mov][0])/2)*(moves[mov][2]-(m_pyramidSize-moves[mov][0])/2) );
        movesScore[mov][currentPlayer][1]=m_pyramidSize-scTmpD;

        //factor altitude
        movesScore[mov][currentPlayer][2]=moves[mov][0];

        //Calcul de la somme pondérée des coefficients pour le score final
        scoreFinalMove[mov]=(movesScore[mov][currentPlayer][0]-movesScore[mov][adverse][0])*AI_FACTOR_BILLES;
        scoreFinalMove[mov]+=movesScore[mov][currentPlayer][1]*AI_FACTOR_CENTER;
        scoreFinalMove[mov]+=movesScore[mov][currentPlayer][2]*AI_FACTOR_ALTITUDE;
        scoreFinalMove[mov]+=(movesScore[mov][currentPlayer][3]-movesScore[mov][adverse][3])*AI_FACTOR_SHAPE;
        scoreFinalMove[mov]+=(movesScore[mov][currentPlayer][4]-movesScore[mov][adverse][4])*AI_FACTOR_OWN_1;
        scoreFinalMove[mov]+=(movesScore[mov][currentPlayer][5]-movesScore[mov][adverse][5])*AI_FACTOR_OWN_2;
        scoreFinalMove[mov]+=(movesScore[mov][currentPlayer][6]-movesScore[mov][adverse][6])*AI_FACTOR_OWN_3;
        scoreFinalMove[mov]+=(movesScore[mov][currentPlayer][7]-movesScore[mov][adverse][7])*AI_FACTOR_OWN_4;
    }


    //Recherche du coup optimal
    double max=-10000000;
    int movMax=0;
    cout<<"Recherche du maximum"<<endl;
    vtkActor *actor=tabSphereActor[0][0][0];
    for(int mov=0;mov<possMoves;mov++){
 /*     Utile pour du profilage des parametres, cette séquence liste les 8 scores obtenus pour chaque coup, et le score issu de la ponderation
     cout<<"Coup numéro "<<mov<<", coord=("<<moves[mov][0]<<","<<moves[mov][1]<<","<<moves[mov][2]<<"), score="<<scoreFinalMove[mov];
        cout<<", Detail : BIL="<<(movesScore[mov][currentPlayer][0]-movesScore[mov][adverse][0]);
        cout<<", CENT="<<movesScore[mov][currentPlayer][1]<<", ALT="<<movesScore[mov][currentPlayer][2];
        cout<<", SHAPE="<<(movesScore[mov][currentPlayer][3]-movesScore[mov][adverse][3]);
        cout<<", OWNS_J1=["<<(movesScore[mov][currentPlayer][4])<<", "<<(movesScore[mov][currentPlayer][5]);
        cout<<", "<<(movesScore[mov][currentPlayer][6])<<", "<<(movesScore[mov][currentPlayer][7])<<"]";
        cout<<", OWNS_DIFF=["<<(movesScore[mov][currentPlayer][4]-movesScore[mov][adverse][4])<<", "<<(movesScore[mov][currentPlayer][5]-movesScore[mov][adverse][5]);
        cout<<", "<<(movesScore[mov][currentPlayer][6]-movesScore[mov][adverse][6])<<", "<<(movesScore[mov][currentPlayer][7]-movesScore[mov][adverse][7])<<"]"<<endl;
*/
        if(scoreFinalMove[mov]>max){
            max=scoreFinalMove[mov];
            movMax=mov;
            actor=tabSphereActor[moves[mov][0]][moves[mov][1]][moves[mov][2]];
        }
     }
    if(verbose)cout<<"AI : coup à jouer : coup numero "<<movMax<<endl<<endl;
    moves.clear();
    scoreFinalMove.clear();
    movesScore.clear();
    mask.clear();
    maskR.clear();
    maskC.clear();
    return actor;
}




int PyramidModel::howMany(int val1,int val2,int val3,int val4,int num){
    int ret=0;
    if(val1==num)ret++;
    if(val2==num)ret++;
    if(val3==num)ret++;
    if(val4==num)ret++;
    return ret;
}


//Une amélioration de cette fonction consisterait à la rendre récursive, pour remplacer le parcours en largeur
int PyramidModel::playThisMarble(vtkActor* actor,int player)
{
    int hashcode=identifyActiveMarble(actor,false);
    if(hashcode==-1)return 0;
    int plev=hashcode/(m_pyramidSize*m_pyramidSize);
    hashcode-=(plev*m_pyramidSize*m_pyramidSize);
    int prow=hashcode/(m_pyramidSize);
    int pcol=hashcode-(prow*m_pyramidSize);

    //Check if the marble is playable. If not, exit, else, own it.
    if(gameIndicators[plev][prow][pcol]!=0)return 0;
    ownedMarble(plev,prow,pcol,player);


    //Actualize grid : recursive search for new possessed marbles
    int nbAdd=1;
    int nbAddTot=1;
    int pass=0;
    int l,r,c,val1,val2,val3,val4,gag,tmp;
    std::vector< std::vector < int > > add;
    std::vector< std::vector < int > > next;
    next.push_back(std::vector<int> (3,0));
    next[0][0]=plev;
    next[0][1]=prow;
    next[0][2]=pcol;


    while(nbAdd>0){
        nbAdd=0;
        pass++;

        //Les next d'avant deviennent les add d'aujourd'hui
        while(add.size()>0)add.pop_back();
        tmp=next.size();
        for(int i =0;i<tmp;i++)add.push_back(next[i]);
        while(next.size()>0)next.pop_back();
        //for each marble added, actualization of new playing possibilities
        for(int ad=0;ad<add.size();ad++)
        {
            l=add[ad][0];
            r=add[ad][1];
            c=add[ad][2];
            //check for emerging marble +r +c
            if(r<m_pyramidSize-l-1 && c<m_pyramidSize-l-1){
                if(gameIndicators[l][r+1][c]>0 && gameIndicators[l][r+1][c+1]>0 && gameIndicators[l][r][c+1]>0 && gameIndicators[l][r][c]>0 &&gameIndicators[l+1][r][c]<=0){
                    val1=gameIndicators[l][r][c];
                    val2=gameIndicators[l][r+1][c];
                    val3=gameIndicators[l][r][c+1];
                    val4=gameIndicators[l][r+1][c+1];
                    gag=gagneCarre(val1,val2,val3,val4,m_nbPlayers);
                    if(gag==0){setNewPlayableMarble(l+1,r,c);}
                    else{
                        nbAdd++;
                        ownedMarble(l+1,r,c,gag-1);
                        next.push_back(std::vector<int>(3,0));
                        next[nbAdd-1][0]=l+1;
                        next[nbAdd-1][1]=r;
                        next[nbAdd-1][2]=c;
                    }
                }
            }

            //check for emerging marble -r +c
            if(r>0 && c<m_pyramidSize-l-1){
                if(gameIndicators[l][r-1][c]>0 && gameIndicators[l][r-1][c+1]>0 && gameIndicators[l][r][c+1]>0 && gameIndicators[l][r][c]>0 &&gameIndicators[l+1][r-1][c]<=0){
                    val1=gameIndicators[l][r][c];
                    val2=gameIndicators[l][r-1][c];
                    val3=gameIndicators[l][r][c+1];
                    val4=gameIndicators[l][r-1][c+1];
                    gag=gagneCarre(val1,val2,val3,val4,m_nbPlayers);
                    if(gag==0){setNewPlayableMarble(l+1,r-1,c);}
                    else{
                        nbAdd++;
                        ownedMarble(l+1,r-1,c,gag-1);
                        next.push_back(std::vector<int>(3,0));
                        next[nbAdd-1][0]=l+1;
                        next[nbAdd-1][1]=r-1;
                        next[nbAdd-1][2]=c;
                    }
                }
            }

             //check for emerging marble +r -c
            if(r<m_pyramidSize-l-1 && c>0){
                if(gameIndicators[l][r+1][c]>0 && gameIndicators[l][r+1][c-1]>0 && gameIndicators[l][r][c-1]>0 && gameIndicators[l][r][c]>0 &&gameIndicators[l+1][r][c-1]<=0){
                     val1=gameIndicators[l][r][c];
                    val2=gameIndicators[l][r+1][c];
                    val3=gameIndicators[l][r][c-1];
                    val4=gameIndicators[l][r+1][c-1];
                    gag=gagneCarre(val1,val2,val3,val4,m_nbPlayers);
                   if(gag==0){
                         setNewPlayableMarble(l+1,r,c-1);
                    }
                    else{
                        nbAdd++;
                        ownedMarble(l+1,r,c-1,gag-1);
                        next.push_back(std::vector<int>(3,0));
                        next[nbAdd-1][0]=l+1;
                        next[nbAdd-1][1]=r;
                        next[nbAdd-1][2]=c-1;
                    }
                }
            }

            //check for emerging marble -r -c
            if(r>0 && c>0){
                if(gameIndicators[l][r-1][c]>0 && gameIndicators[l][r-1][c-1]>0 && gameIndicators[l][r][c-1]>0 && gameIndicators[l][r][c]>0 &&gameIndicators[l+1][r-1][c-1]<=0){
                    val1=gameIndicators[l][r][c];
                    val2=gameIndicators[l][r-1][c];
                    val3=gameIndicators[l][r][c-1];
                    val4=gameIndicators[l][r-1][c-1];
                    gag=gagneCarre(val1,val2,val3,val4,m_nbPlayers);
                    if(gag==0){setNewPlayableMarble(l+1,r-1,c-1);}
                    else{
                        nbAdd++;
                        ownedMarble(l+1,r-1,c-1,gag-1);
                        next.push_back(std::vector<int>(3,0));
                        next[nbAdd-1][0]=l+1;
                        next[nbAdd-1][1]=r-1;
                        next[nbAdd-1][2]=c-1;
                    }
                }
            }
        }
        nbAddTot+=nbAdd;
    }
    m_gmw->m_mouseInteractor->StopState();
    return nbAddTot;
}


void PyramidModel::ownedMarble(int l,int r,int c,int player){
    if(gameIndicators[l][r][c]!=0) setNewPlayableMarble(l,r,c);
    (m_gmw->m_playersScores[player])++;
    if(player==0)(tabSphereActor[l][r][c])->GetProperty()->SetColor(0.6+0.4*(l+1)/(1.0*m_pyramidSize), 0, 0); //(R,G,B)
    if(player==1)(tabSphereActor[l][r][c])->GetProperty()->SetColor(0, 0, 0.6+0.4*(l+1)/(1.0*m_pyramidSize)); //(R,G,B)
    (tabSphereActor[l][r][c])->GetProperty()->SetOpacity(1); //(R,G,B)
    (tabSphereActor[l][r][c])->GetProperty()->SetInterpolationToPhong(); //(R,G,B)
    gameIndicators[l][r][c]=player+1;
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    m_gmw->m_mouseInteractor->StopState();
    if(l>m_currentStage){
        m_currentStage=l;
        m_gmw->m_renderer->ResetCamera();
    }
}

int PyramidModel::gagneCarre(int val1,int val2, int val3, int val4,int nbPlayers){
    std::vector <int> counts=std::vector<int>(nbPlayers+1,0);
    counts[val1]++;
    counts[val2]++;
    counts[val3]++;
    counts[val4]++;
    if(nbPlayers==2){
        if(counts[1]>=3)return 1;
        if(counts[2]>=3)return 2;
        return 0;
    }
    else {
        int max1=0;
        int max2=0;
        int ind=0;
        for(int i =0;i<nbPlayers+1 ; i++){
            if(counts[i]>=max1){
                max2=max1;
                max1=counts[i];
                ind=i;
            }
        }
        if(max1>=2 && max2!=2)return ind;
        return 0;
    }
}


