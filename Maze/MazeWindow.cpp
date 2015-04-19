/************************************************************************
     File:        MazeWindow.cpp

     Author:     
                  Stephen Chenney, schenney@cs.wisc.edu
     Modifier
                  Yu-Chi Lai, yu-chi@cs.wisc.edu

     Comment:    
						(c) 2001-2002 Stephen Chenney, University of Wisconsin at Madison

						Class header file for the MazeWindow class. The MazeWindow is
						the window in which the viewer's view of the maze is displayed.
		

     Platform:    Visio Studio.Net 2003 (converted to 2005)

*************************************************************************/

#include "MazeWindow.h"
#include <Fl/math.h>
#include <Fl/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include "LineSeg.h"
#include "Edge.h"


//*************************************************************************
//
// * Constructor
//=========================================================================
MazeWindow::
MazeWindow(int x, int y, int width, int height, char *label,Maze *m)
	: Fl_Gl_Window(x, y, width, height, label)
//=========================================================================
{
	maze = m;

	// The mouse button isn't down and there is no key pressed.
	down = false;
	z_key = 0;
}


//*************************************************************************
//
// * Set the maze. Also causes a redraw.
//=========================================================================
void MazeWindow::
Set_Maze(Maze *m)
//=========================================================================
{
	// Change the maze
	maze = m;

	// Force a redraw
	redraw();
}
void MazeWindow::draw_cell(Cell* drawCell, Edge viewLineR, Edge viewLineL, float focal_length){
	float gCos=cos(Maze::To_Radians(maze->viewer_dir));
	float gSin=sin(Maze::To_Radians(maze->viewer_dir));
	float mMat[16]={//model
		gCos,	0,	-gSin,	-maze->viewer_posn[1]*gCos-maze->viewer_posn[0]*-gSin,
		0,		1,	0,		-maze->viewer_posn[2],
		-gSin,	0,	-gCos,	-maze->viewer_posn[1]*-gSin-maze->viewer_posn[0]*-gCos,
		0,		0,	0,		1
	};
	for(int edgeIndex=0;edgeIndex<4;edgeIndex++){
		float crossParR1=LineSeg(&viewLineR).Cross_Param(LineSeg(drawCell->edges[edgeIndex]));
		float crossParR2=LineSeg(drawCell->edges[edgeIndex]).Cross_Param(LineSeg(&viewLineR));
		float crossParL1=LineSeg(&viewLineL).Cross_Param(LineSeg(drawCell->edges[edgeIndex]));
		float crossParL2=LineSeg(drawCell->edges[edgeIndex]).Cross_Param(LineSeg(&viewLineL));
		float p[2][2];
		p[0][0]=drawCell->edges[edgeIndex]->endpoints[0]->posn[0];
		p[0][1]=drawCell->edges[edgeIndex]->endpoints[0]->posn[1];
		p[1][0]=drawCell->edges[edgeIndex]->endpoints[1]->posn[0];
		p[1][1]=drawCell->edges[edgeIndex]->endpoints[1]->posn[1];
		int sideRS=viewLineR.Point_Side(p[0][0],p[0][1]);
		int sideRE=viewLineR.Point_Side(p[1][0],p[1][1]);
		int sideLS=viewLineL.Point_Side(p[0][0],p[0][1]);
		int sideLE=viewLineL.Point_Side(p[1][0],p[1][1]);
		bool crossR=0.<crossParR1&&0.<crossParR2&&crossParR2<1.;
		bool crossL=0.<crossParL1&&0.<crossParL2&&crossParL2<1.;
		bool includeRL=(sideRS==0||sideRS==2)&&(sideRE==0||sideRE==2)&&(sideLS==1||sideLS==2)&&(sideLE==1||sideLE==2);
		if((crossR||crossL||includeRL)&&drawCell->edges[edgeIndex]->drawFram!=maze->frame_num){
			//clip wall
			if(crossR&&crossL){
				if(sideRS==1||sideRS==2){
					float tp[2]={p[0][0],p[0][1]};
					p[0][0]=p[0][0]+(p[1][0]-p[0][0])*crossParR2;
					p[0][1]=p[0][1]+(p[1][1]-p[0][1])*crossParR2;

					p[1][0]=tp[0]+(p[1][0]-tp[0])*crossParL2;
					p[1][1]=tp[1]+(p[1][1]-tp[1])*crossParL2;
				}
				else{
					float tp[2]={p[1][0],p[1][1]};
					p[1][0]=p[0][0]+(p[1][0]-p[0][0])*crossParR2;
					p[1][1]=p[0][1]+(p[1][1]-p[0][1])*crossParR2;

					p[0][0]=p[0][0]+(tp[0]-p[0][0])*crossParL2;
					p[0][1]=p[0][1]+(tp[1]-p[0][1])*crossParL2;
				}
			}
			if(crossR&&!crossL){
				if(sideRS==0||sideRS==2&&sideRE==1){
					p[1][0]=p[0][0]+(p[1][0]-p[0][0])*crossParR2;
					p[1][1]=p[0][1]+(p[1][1]-p[0][1])*crossParR2;
				}
				else{
					p[0][0]=p[0][0]+(p[1][0]-p[0][0])*crossParR2;
					p[0][1]=p[0][1]+(p[1][1]-p[0][1])*crossParR2;
				}
			}
			if(!crossR&&crossL){
				if(sideLS==1||sideLS==2&&sideLE==0){
					p[1][0]=p[0][0]+(p[1][0]-p[0][0])*crossParL2;
					p[1][1]=p[0][1]+(p[1][1]-p[0][1])*crossParL2;
				}
				else{
					p[0][0]=p[0][0]+(p[1][0]-p[0][0])*crossParL2;
					p[0][1]=p[0][1]+(p[1][1]-p[0][1])*crossParL2;
				}
			}
			if((p[0][0]-p[1][0])*(p[0][0]-p[1][0])<0.000000001&&(p[0][1]-p[1][1])*(p[0][1]-p[1][1])<0.000000001)
				continue;
			//draw wall
			drawCell->edges[edgeIndex]->drawFram=maze->frame_num;
			if (drawCell->edges[edgeIndex]->opaque) {
				float sp[4][4]={
					{p[0][1], 1,p[0][0],1},
					{p[0][1],-1,p[0][0],1},
					{p[1][1], 1,p[1][0],1},
					{p[1][1],-1,p[1][0],1}
				};
				float sp2[4][4];
				for(int i=0;i<4;i++){
					for(int j=0;j<4;j++){
						sp2[i][j]=mMat[j*4+0]*sp[i][0]+mMat[j*4+1]*sp[i][1]+mMat[j*4+2]*sp[i][2]+mMat[j*4+3]*sp[i][3];
					}
				}
				for(int i=0;i<4;i++){
					sp2[i][0]*=focal_length/sp2[i][2];
					sp2[i][1]*=focal_length/sp2[i][2];
				}
				glColor3fv(drawCell->edges[edgeIndex]->color);
				glBegin(GL_QUADS);
				glColor3fv(drawCell->edges[edgeIndex]->color);
				glVertex2fv(sp2[0]);
				glVertex2fv(sp2[1]);
// 				glColor3fv(drawCell->edges[edgeIndex]->color);
// 				glVertex2f((sp2[1][0]+sp2[3][0])/2,(sp2[1][1]+sp2[3][1])/2);
// 				glVertex2f((sp2[0][0]+sp2[2][0])/2,(sp2[0][1]+sp2[2][1])/2);
// 				glVertex2f((sp2[0][0]+sp2[2][0])/2,(sp2[0][1]+sp2[2][1])/2);
// 				glVertex2f((sp2[1][0]+sp2[3][0])/2,(sp2[1][1]+sp2[3][1])/2);
// 				glColor3f(1,1,1);
				glVertex2fv(sp2[3]);
				glVertex2fv(sp2[2]);
				glEnd();
			}
			else{
				if(drawCell->edges[edgeIndex]->Neighbor(drawCell)){
					Vertex viewPointO(0,maze->viewer_posn[0],maze->viewer_posn[1]);
					Vertex nextViewPointR(0,p[1][0],p[1][1]);
					Vertex nextViewPointL(0,p[0][0],p[0][1]);
					if((crossParR1>0&&crossParL1>0&&crossParR2<crossParL2)||(!(crossParR1>0&&crossParL1>0)&&crossParR2>crossParL2)){
						nextViewPointR.posn[0]=p[0][0];
						nextViewPointR.posn[1]=p[0][1];
						nextViewPointL.posn[0]=p[1][0];
						nextViewPointL.posn[1]=p[1][1];
					}
					Edge nextViewLineR(0,&viewPointO,&nextViewPointR,0,0,0);
					Edge nextViewLineL(0,&viewPointO,&nextViewPointL,0,0,0);
					draw_cell(drawCell->edges[edgeIndex]->Neighbor(drawCell), nextViewLineR, nextViewLineL, focal_length);
				}
			}
		}
	}
}

//*************************************************************************
//
// * draw() method invoked whenever the view changes or the window
//   otherwise needs to be redrawn.
//=========================================================================
void MazeWindow::
draw(void)
//=========================================================================
{
	float   focal_length;

	if ( ! valid() ) {
		// The OpenGL context may have been changed
		// Set up the viewport to fill the window.
		glViewport(0, 0, w(), h());

		// We are using orthogonal viewing for 2D. This puts 0,0 in the
		// middle of the screen, and makes the image size in view space
		// the same size as the window.
		gluOrtho2D(-w() * 0.5, w() * 0.5, -h() * 0.5, h() * 0.5);

		// Sets the clear color to black.
		glClearColor(0.0, 0.0, 0.0, 1.0);
	}

	// Clear the screen.
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_QUADS);
		// Draw the "floor". It is an infinite plane perpendicular to
		// vertical, so we know it projects to cover the entire bottom
		// half of the screen. Walls of the maze will be drawn over the top
		// of it.
		glColor3f(0.2f, 0.2f, 0.2f);
		glVertex2f(-w() * 0.5f, -h() * 0.5f);
		glVertex2f( w() * 0.5f, -h() * 0.5f);
		glVertex2f( w() * 0.5f, 0.0       );
		glVertex2f(-w() * 0.5f, 0.0       );

		// Draw the ceiling. It will project to the entire top half
		// of the window.
		glColor3f(0.4f, 0.4f, 0.4f);
		glVertex2f( w() * 0.5f,  h() * 0.5f);
		glVertex2f(-w() * 0.5f,  h() * 0.5f);
		glVertex2f(-w() * 0.5f, 0.0       );
		glVertex2f( w() * 0.5f, 0.0       );
	glEnd();


	if ( maze ) {
		// Set the focal length. We can do this because we know the
		// field of view and the size of the image in view space. Note
		// the static member function of the Maze class for converting
		// radians to degrees. There is also one defined for going backwards.
		focal_length = w() / (float)(2.0*tan(Maze::To_Radians(maze->viewer_fov)*0.5));
	
		// Draw the 3D view of the maze (the visible walls.) You write this.
		// Note that all the information that is required to do the
		// transformations and projection is contained in the Maze class,
		// plus the focal length.
		maze->Draw_View(focal_length);

		Vertex viewPointO(0,maze->viewer_posn[0],maze->viewer_posn[1]);
		Vertex viewPointR(0,maze->viewer_posn[0]+focal_length*cos(Maze::To_Radians(maze->viewer_dir-maze->viewer_fov/2)),maze->viewer_posn[1]+focal_length*sin(Maze::To_Radians(maze->viewer_dir-maze->viewer_fov/2)));
		Vertex viewPointL(0,maze->viewer_posn[0]+focal_length*cos(Maze::To_Radians(maze->viewer_dir+maze->viewer_fov/2)),maze->viewer_posn[1]+focal_length*sin(Maze::To_Radians(maze->viewer_dir+maze->viewer_fov/2)));
		Edge viewLineR(0,&viewPointO,&viewPointR,0,0,0);
		Edge viewLineL(0,&viewPointO,&viewPointL,0,0,0);
		draw_cell(maze->view_cell, viewLineR, viewLineL, focal_length);
	}
}


//*************************************************************************
//
// *
//=========================================================================
bool MazeWindow::
Drag(float dt)
//=========================================================================
{
	float   x_move, y_move, z_move;

	if ( down ) {
		int	dx = x_down - x_last;
		int   dy = y_down - y_last;
		float dist;

		// Set the viewing direction based on horizontal mouse motion.
		maze->Set_View_Dir(d_down + 360.0f * dx / (float)w());

		// Set the viewer's linear motion based on a speed (derived from
		// vertical mouse motion), the elapsed time and the viewing direction.
		dist = 10.0f * dt * dy / (float)h();
		x_move = dist * (float)cos(Maze::To_Radians(maze->viewer_dir));
		y_move = dist * (float)sin(Maze::To_Radians(maze->viewer_dir));
	}
	else {
		x_move = 0.0;
		y_move = 0.0;
	}

	// Update the z posn
	z_move = z_key * 0.1f;
	z_key = 0;

	// Tell the maze how much the view has moved. It may restrict the motion
	// if it tries to go through walls.
	maze->Move_View_Posn(x_move, y_move, z_move);

	return true;
}


//*************************************************************************
//
// *
//=========================================================================
bool MazeWindow::
Update(float dt)
//=========================================================================
{
	// Update the view

	if ( down || z_key ) // Only do anything if the mouse button is down.
		return Drag(dt);

	// Nothing changed, so no need for a redraw.
	return false;
}


//*************************************************************************
//
// *
//=========================================================================
int MazeWindow::
handle(int event)
//=========================================================================
{
	if (!maze)
		return Fl_Gl_Window::handle(event);

	// Event handling routine.
	switch ( event ) {
		case FL_PUSH:
			down = true;
			x_last = x_down = Fl::event_x();
			y_last = y_down = Fl::event_y();
			d_down = maze->viewer_dir;
			return 1;
		case FL_DRAG:
			x_last = Fl::event_x();
			y_last = Fl::event_y();
			return 1;
		case FL_RELEASE:
			down = false;
			return 1;
		case FL_KEYBOARD:
			if(Fl::event_key()=='w'||Fl::event_key()=='a'||Fl::event_key()=='s'||Fl::event_key()=='d'){
				down = true;
				x_down = 0;
				y_down = 0;
				x_last = (Fl::event_key()=='d')*10-(Fl::event_key()=='a')*10;
				y_last = (Fl::event_key()=='s')*10-(Fl::event_key()=='w')*10;
				d_down = maze->viewer_dir;
				Drag(1);
				down = false;
				this->flush();
				return 1;
			}
		case FL_FOCUS:
		case FL_UNFOCUS:
			return 1;
	}

	// Pass any other event types on the superclass.
	return Fl_Gl_Window::handle(event);
}


