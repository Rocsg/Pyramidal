#include <QApplication>
#include "GameMainWindow.h"




void test(){



}




int main( int argc, char** argv )
{
    bool doTest=false;
    if(doTest){
        test();
        return 0;
    }
    else{
        QApplication app( argc, argv );
        GameMainWindow gameMainWindow;
        gameMainWindow.show();
        return app.exec();
    }
}
