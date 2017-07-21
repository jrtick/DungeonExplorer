#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <vector>
#include "Texture.h"
#include "Sounds.h"

inline float unitRandom(){
  return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}


#define min(x,y) ((x<y)? x : y)
#define max(x,y) ((x>y)? x : y)

class Thing{
public:
  static std::vector<Thing*> thingList;
  virtual void draw(Point* viewport){ }
  Point bbox[2];
  int idx = -1;
  Thing(bool collideable = true){
    if(collideable){
      idx = thingList.size();
      thingList.push_back(this);
    }
  }
  int Collision(){
    for(int i=0;i<(int)thingList.size();i++){
      if(i == idx) continue;
      else if(collides(this,thingList[i])) return i;
    }
  }
  static bool collides(Thing* thing1, Thing* thing2){
    //AABB collision detection
    Point bbox1[2] = thing1->bbox;
    Point bbox2[2] = thing2->bbox;
    return (bbox1[0].x < bbox2[1].x && bbox1[1].x > bbox2[0].x && 
            bbox1[0].y < bbox2[1].y && bbox1[1].y > bbox2[0].y);
  }
};
std::vector<Thing*> Thing::thingList; //initialize

class MoveableThing : public Thing{
public:
  MoveableThing() : Thing(){ }
  SpriteSheet spritesheet;
  void draw(Point* viewport){
    Point min = viewport[0],
          max = viewport[1];
    Point scale = Point(1.f/(max.x-min.x),1.f/(max.y-min.y));
    spritesheet.draw((bbox[0]                   -min)*scale*2-1,
                     (Point(bbox[1].x,bbox[0].y)-min)*scale*2-1,
                     (bbox[1]                   -min)*scale*2-1,
                     (Point(bbox[0].x,bbox[1].y)-min)*scale*2-1);
  }
};
class StationaryThing : public Thing{
public:
  StationaryThing() : Thing(){ }
  Texture texture;
  void draw(Point* viewport){
    Point min = viewport[0],
           max = viewport[1];
    Point scale = Point(1.f/(max.x-min.x),1.f/(max.y-min.y));
    texture.drawQuad((bbox[0]                   -min)*scale*2-1,
                     (Point(bbox[1].x,bbox[0].y)-min)*scale*2-1,
                     (bbox[1]                   -min)*scale*2-1,
                     (Point(bbox[0].x,bbox[1].y)-min)*scale*2-1);
  }
};

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


class Room: public Thing{
public:
  static Texture brick;
  Room() : Thing(false){ }
  void drawRoom(Point* viewport, bool tile=false){
    Point min = viewport[0],
           max = viewport[1];
    Point scale = Point(1.f/(max.x-min.x),1.f/(max.y-min.y));
    Point corners[4];
    corners[0] = (bbox[0]-min)*scale*2-1;
    corners[1] = (Point(bbox[1].x,bbox[0].y)-min)*scale*2-1;
    corners[2] = (bbox[1]-min)*scale*2-1;
    corners[3] = (Point(bbox[0].x,bbox[1].y)-min)*scale*2-1;
    brick.drawQuad(corners[0],corners[1],corners[2],corners[3],10);
  }
  void draw(Point* viewport){
    drawRoom(viewport);
  }
};
Texture Room::brick; //empty initialization


class Board{
public:
  int numRooms = 0;
  Room* rooms = NULL;
  Point minCoord = Point(0,0), maxCoord = Point(100,100);
  Board(){}
  Board(int roomCount, bool autoGenerate=true){
    numRooms = roomCount;
    rooms = (Room*) malloc(sizeof(Room)*numRooms);
    if(autoGenerate && roomCount>0){
      Point p1 = minCoord + (maxCoord-minCoord)*Point(unitRandom(),unitRandom()),
            p2 = minCoord + (maxCoord-minCoord)*Point(unitRandom(),unitRandom());
      rooms[0].bbox[0] = Point(min(p1.x,p2.x),min(p1.y,p2.y));
      rooms[0].bbox[1] = Point(max(p1.x,p2.x),max(p1.y,p2.y));
      for(int i=1;i<roomCount;i++){ //attach new room to prev room
        Point minPt = rooms[i-1].bbox[0],
              maxPt = rooms[i-1].bbox[1];
        p1 = minPt+(maxPt-minPt)*Point(unitRandom(),unitRandom());
        p2 = minCoord + (maxCoord-minCoord)*Point(unitRandom(),unitRandom());
        rooms[i].bbox[0] = Point(min(p1.x,p2.x),min(p1.y,p2.y));
        rooms[i].bbox[1] = Point(max(p1.x,p2.x),max(p1.y,p2.y));
      }

      for(int i=0;i<roomCount;i++)
        printf("Room %d: (%.2f,%.2f)<->(%.2f,%.2f)\n",i,rooms[i].bbox[0].x,rooms[i].bbox[0].y,rooms[i].bbox[1].x,rooms[i].bbox[1].y);
    }
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
  bool inBounds(Thing* thing){
    for(int i=0;i<numRooms;i++){
      if(Thing::collides(thing,&rooms[i])) return true;
    }
    return false;
  }
};

struct GameState{
  bool paused;
  Point viewport[2];
  MoveableThing player;
  std::vector<StationaryThing> loot;
  std::vector<MoveableThing> enemies;
  int frameCount = 0;
  double fps = -1;
  bool fullscreen = false;
  int resolution[2] = {512,512};
  std::clock_t starttime, lastCheck;
  Board board;
  GameState(bool paused=true){
    starttime = lastCheck = std::clock();
    this->paused = paused;
    viewport[0] = Point(0,0);
    viewport[1] = Point(100,100);
    player.bbox[0] = Point(40,40);
    player.bbox[1] = Point(60,60);
  }
};

GameState curState = GameState(true);

void drawText(Point p, void* font, char* msg, V4 color){
  glColor4f(color.x,color.y,color.z,color.w);
  glRasterPos2f(p.x,p.y);
  glutBitmapString(font,(unsigned char*)msg);
  glColor4f(1,1,1,1); //set back to default
}

void drawLoot(){
  for(unsigned int i=0;i<curState.loot.size();i++){
    curState.loot[i].draw((Point*)curState.viewport);
  }
}
void drawMonsters(){
  for(unsigned int i=0;i<curState.enemies.size();i++){
    curState.enemies[i].draw((Point*)curState.viewport);
  }
}
/*
void drawPlayer(Point* viewport){
    Point min = viewport[0],
           max = viewport[1];
    Point scale = Point(1.f/(max.x-min.x),1.f/(max.y-min.y));
    Point p1,p2,p3,p4;
    p1 = p2 = p3 = p4 = curState.playerPosition;
    p1 += Point(-curState.playerRadius,-curState.playerRadius);
    p2 += Point(curState.playerRadius,-curState.playerRadius);
    p3 += Point(curState.playerRadius,curState.playerRadius);
    p4 += Point(-curState.playerRadius,curState.playerRadius);
    curState.player.draw((p1-min)*scale*2-1,(p2-min)*scale*2-1,(p3-min)*scale*2-1,(p4-min)*scale*2-1);
}
*/
void drawGUI(){
  char buf[128];
  sprintf(buf,"FPS: %.2f",curState.fps);
  drawText(Point(-1,1-50.f/float(curState.resolution[1])),GLUT_BITMAP_TIMES_ROMAN_24, (char*)buf, V4(0,0,1,1));

  if(curState.paused){
    sprintf(buf,"Game Paused. press P to unpause");
    drawText(Point(-0.5,0),GLUT_BITMAP_TIMES_ROMAN_24,(char*)buf,V4(1,0,0,1));
  }
}

#define FRAMES_TO_AVERAGE 300
void draw(void) {
  glClear(GL_COLOR_BUFFER_BIT);

  if(!curState.paused){//continue game
    curState.board.drawBoard((Point*)curState.viewport);
    drawLoot();
    drawMonsters();
    if(!curState.board.inBounds(&curState.player)){glColor3f(1,0,0);}
    curState.player.draw((Point*)curState.viewport);//drawPlayer((Point*)curState.viewport);
  }
  drawGUI();

  //calculate framerate
  if((curState.frameCount++ % FRAMES_TO_AVERAGE) == 0){
    std::clock_t curtime = std::clock();
    double timePassed = (curtime - curState.lastCheck) / (double) CLOCKS_PER_SEC;
    curState.fps = ((double) FRAMES_TO_AVERAGE) / timePassed;
    curState.lastCheck = curtime;
  }
  glutSwapBuffers(); //display to window
}

#define ESC 27
void keyPressedPaused(unsigned char key, int mouseX, int mouseY){
  switch(key){
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
      printf("Warning: %c (%d) key not recognized in pause mode.\n",key,(int)key);
  }
}
void keyPressed(unsigned char key, int mouseX, int mouseY){
  if(curState.paused) return keyPressedPaused(key,mouseX,mouseY);
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
    case 'w':
      curState.player.bbox[0].y++;
      curState.player.bbox[1].y++;
      break;
    case 's':
      curState.player.bbox[0].y--;
      curState.player.bbox[1].y--;
      break;
    case 'a':
      curState.player.bbox[0].x--;
      curState.player.bbox[1].x--;
      break;
    case 'd':
      curState.player.bbox[0].x++;
      curState.player.bbox[1].x++;
      break;
    case 'r':
      curState.board = Board(int(unitRandom()*20));
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
  
  srand((int)std::clock()); //init rand seed
  
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
  Room::brick = Texture((char*)"textures/brick.jpg");
  Texture gold = Texture((char*)"textures/gold.png");
  //curState.brick2 = Texture((char*)"textures/brick.png");
  curState.player.spritesheet = SpriteSheet((char*)"textures/player.png",1,5);
;
  int animation[5] = {0,1,2,3,4};
  curState.player.spritesheet.setAnimation(curState.player.spritesheet.addAnimation((int*)animation, 5,50));
  Room::brick.activate(); //set default

  curState.board = Board(5); //init board
  int numTreasure = int(unitRandom()*40); //init loot
  for(int i=0;i<numTreasure;i++){
    int roomIdx = int(unitRandom()*curState.board.numRooms);
    Point min = curState.board.rooms[roomIdx].bbox[0],
          max = curState.board.rooms[roomIdx].bbox[1];
    StationaryThing loot;
    loot.texture = gold;
    Point c = min+(max-min)*Point(unitRandom(),unitRandom());
    loot.bbox[0] = c-5;
    loot.bbox[1] = c+5;
    curState.loot.push_back(loot);
  }
  int numEnemies = int(unitRandom()*10);
  for(int i=0;i<numEnemies;i++){ //init enemies
    int roomIdx = int(unitRandom()*curState.board.numRooms);
    Point min = curState.board.rooms[roomIdx].bbox[0],
          max = curState.board.rooms[roomIdx].bbox[1];
    MoveableThing enemy;
    enemy.spritesheet = curState.player.spritesheet;
    Point c = min+(max-min)*Point(unitRandom(),unitRandom());
    enemy.bbox[0] = c-5;
    enemy.bbox[1] = c+5;
    curState.enemies.push_back(enemy);
  }

  //setup callbacks
  glutReshapeFunc(onResize);
  glutKeyboardFunc(keyPressed);
  glutSpecialFunc(specialKeyPressed);
  glutDisplayFunc(draw);
  glutIdleFunc(draw);

  Sound::init(argc,argv);
  Sound sound = Sound((char*)"sounds/ask_mr_hat.wav");
  sound.play(true);
  Sound::cleanup();

  glutMainLoop();
  return 0;
}


//gcc dungeon.cpp -o exec -lGL -lGLU -lglut
