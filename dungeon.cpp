#include <GL/freeglut.h>
#include <stdio.h>
#include <ctime>
#include "Texture.h"



struct GameState{
  bool paused;
  Point viewport[2];
  Texture brick1,brick2;
  SpriteSheet player;
  int frameCount = 0;
  double fps = -1;
  bool fullscreen = false;
  int resolution[2] = {1024,1024};
  std::clock_t starttime, lastCheck;
  GameState(bool paused){
    starttime = lastCheck = std::clock();
    this->paused = paused;
    viewport[0] = Point(0,0);
    viewport[1] = Point(100,100);
  }
};
GameState curState = GameState(true);

class Room{
public:
  Point bbox[4];
  Room(){ }
  void drawRoom(Point* viewport, bool tile=false){
    Point min = viewport[0],
           max = viewport[1];
    Point scale = Point(1.f/(max.x-min.x),1.f/(max.y-min.y));
    Point corners[4];
    for(int i=0;i<4;i++)
      corners[i] = Point(2*(bbox[i].x-min.x)*scale.x-1,
                         2*(bbox[i].y-min.y)*scale.y-1);
    curState.brick1.drawQuad(corners[0],corners[1],corners[2],corners[3],10);
  }
};

class Board{
public:
  int numRooms = 0;
  Room* rooms = NULL;
  Board(){}
  Board(int roomCount){
    numRooms = roomCount;
    rooms = (Room*) malloc(sizeof(Room)*numRooms);
  }
  void setRoom(int roomIdx, Point* bbox){
    Point* coords = (Point*)&rooms[roomIdx].bbox;
    for(int i=0;i<4;i++) coords[i] = bbox[i];
  }
  void drawBoard(Point* viewport){
    for(int i=0;i<numRooms;i++){
      rooms[i].drawRoom(viewport);
    }
  }
};
Board board;

struct V4{
  float x,y,z,w;
  V4(){}
  V4(float x, float y, float z, float w){
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
  }
};
void drawText(Point p, void* font, char* msg, V4 color){
  glColor4f(color.x,color.y,color.z,color.w);
  glRasterPos2f(p.x,p.y);
  glutBitmapString(font,(unsigned char*)msg);
  glColor4f(1,1,1,1); //set back to default
}

void drawLoot(){}
void drawMonsters(){}
void drawPlayer(Point* viewport){
    //Point min = viewport[0],
    //       max = viewport[1];
    //Point scale = Point(1.f/(max.x-min.x),1.f/(max.y-min.y));
    curState.player.drawQuad(Point(-0.2,-0.2),Point(0.2,-0.2),Point(0.2,0.2),Point(-0.2,0.2),(curState.frameCount/5)%4);
}
void drawGUI(){
  char buf[128];
  sprintf(buf,"FPS: %.2f",curState.fps);
  drawText(Point(-1,1-30.f/float(curState.resolution[1])),GLUT_BITMAP_TIMES_ROMAN_24, (char*)buf, V4(1,0,0,1));
}

#define FRAMES_TO_AVERAGE 300
void draw(void) {
  glClear(GL_COLOR_BUFFER_BIT);

    board.drawBoard((Point*)curState.viewport);
    drawLoot();
    drawMonsters();
    drawPlayer((Point*)curState.viewport);
    drawGUI();
    if((curState.frameCount++ % FRAMES_TO_AVERAGE) == 0){
      std::clock_t curtime = std::clock();
      double timePassed = (curtime - curState.lastCheck) / (double) CLOCKS_PER_SEC;
      curState.fps = ((double) FRAMES_TO_AVERAGE) / timePassed;
      curState.lastCheck = curtime;
    }
  glutSwapBuffers();
}

#define ESC 27
void keyPressed(unsigned char key, int mouseX, int mouseY){
  switch(key){
    case 'f':
      curState.fullscreen = !curState.fullscreen;
      if(curState.fullscreen){
        printf("fullscreening...\n");
        glutFullScreen();
      }else{
        printf("undoing fullscreen.\n");
        glutReshapeWindow(curState.resolution[0],curState.resolution[1]);
      }
      break;
    case 'q':
    case 'Q':
    case ESC:
      printf("Exiting program.\n");
      exit(0);
      break;
    case 'p':
      curState.paused = !curState.paused;
      break;
    default:
      printf("Warning: %c (%d) key not recognized.\n",key,(int)key);
  }
}
void specialKeyPressed(int key, int mouseX, int mouseY){
  switch(key){
    case GLUT_KEY_LEFT:
    case GLUT_KEY_RIGHT:
    case GLUT_KEY_UP:
    case GLUT_KEY_DOWN:
      printf("Arrow key pressed.\n");
      break;
    default:
      printf("Warning: special key %d not recognized\n",key);
  }
}

void onResize(int width, int height){
  printf("Request to resize to (%d,%d) resolution\n",width,height);
  glViewport(0,0,width,height);
  //glOrtho(0,0,resolution[0],resolution[1],-1,1);
}


//Main program
//typedef unsigned int uint;
int main(int argc, char **argv) {
  glutInit(&argc, argv); 
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

  
  //glutInitWindowPosition(50, 25);
  glutInitWindowSize(curState.resolution[0],curState.resolution[1]);
  glutCreateWindow("Dungeon");
  
  //set Background color
  glEnable(GL_BLEND);
  glEnable(GL_ALPHA_TEST);
  //glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0, 0, 0, 1);

  //load images
  curState.brick1 = Texture((char*)"textures/brick.jpg");
  curState.brick2 = Texture((char*)"textures/brick.png");
  curState.player = SpriteSheet((char*)"textures/player.png",1,5);
  curState.brick1.activate(); //set default

  board = Board(1);
  Point bbox[4];
  bbox[0] = Point(0,0);
  bbox[1] = Point(100,0);
  bbox[2] = Point(100,100);
  bbox[3] = Point(0,100);
  board.setRoom(0,(Point*)bbox);

  //setup callbacks
  glutReshapeFunc(onResize);
  glutKeyboardFunc(keyPressed);
  glutSpecialFunc(specialKeyPressed);
  glutDisplayFunc(draw);
  glutIdleFunc(draw);

  glutMainLoop();
  return 0;
}


//gcc dungeon.cpp -o exec -lGL -lGLU -lglut
