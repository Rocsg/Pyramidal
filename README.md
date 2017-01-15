# Pyramidal

SUMMARY :
1) The Project ------
2) Dependances ------
3) Installation -----



----- The Project ------------------------------------
Pyramidal is a strategy 3d board game. The goal is to play each marble strategically to own the bigger part of the final play board. A marble at upper levels can be played as soon as the 4 underneath marbles are played. When a square of 4 owned marbles is built, if you own 3 of these 4 marbles, you automatically own the upper one as a bonus. This can recursively lead you to take the advantage on the game. 
This game was created by algorithmic curiosity (explore the best way to play through an AI), but you can easily have fun with it.

The current version let one player fighting against the AI. The player can choose between "difficult" (player begins) or "very difficult" (AI begins)
AI is very strong, but it's just a baby : the parameters have been set intuitively. Next step should be take advantage of machine learning in order to tune parameters, and make it becoming unfightable !


----- Dependances ------------------------------------
The project is based on VTK for visualisation purpose. The windowing and interaction system is handled by Qt and QVTK for the rendering context.
For installation under Linux with your package system, the packages should be (assuming MAJOR and MINOR versioning number) : 
vtkMAJOR.MINOR
libvtkMAJOR.MINOR
libvtkMAJOR-qtMAJOR
libvtkMAJOR-qtMAJOR-dev
libvtkMAJOR-dev (in order that "FIND_VTK" works)
qtMAJOR

Suggested versions :
VTK >= 6
QT = 5
 
You might also prefer to build vtk and qt from the sources.


----- Installation ------------------------------------
Use cmake for generating the makefiles, then make for building the application. 
There's no "make install" rules defined.

