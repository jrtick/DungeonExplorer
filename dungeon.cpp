#include <GL/glut.h>
#include <stdio.h>
#include "ImageLoader.h"

void checkError(){
  GLenum errCode = glGetError();
  if(errCode != GL_NO_ERROR){
    printf("Error: %s\n",(char*) gluErrorString(errCode));
  }
}

class Texture{
public:
  static int textureCount;
  GLuint textureBinding;
  int width,height,textureIdx,channels;

  Texture(){ //default constructor does nothing
    textureBinding = -1;
  }

  Texture(char* filename){
    Image image = readImage(filename);
    if(image.numComponents < 0){
      printf("Image load failed\n");
      textureBinding = -1;
      return;
    }else{
      width = image.width;
      height = image.height;
      channels = (char)image.numComponents;
    
      textureIdx = Texture::textureCount++;
      printf("%s has texture idx %d w/ res (%d,%d,%d)\n",filename,textureIdx,width,height,channels);
      ///glActiveTexture(GL_TEXTURE0+textureIdx);

      glGenTextures(1,&textureBinding);
      glBindTexture(GL_TEXTURE_2D, textureBinding);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE , GL_MODULATE);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //when scaling down
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //when scaling up
      
      glTexImage2D(GL_TEXTURE_2D, 0, (channels==3)? GL_RGB8 : GL_RGBA8,width,height,0,
                    (channels==3)? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, image.data);

      /*gluBuild2DMipmaps(GL_TEXTURE_2D,
          (channels==3)? GL_RGB8 : GL_RGBA8,width,height,
          (channels==3)? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, image.data);*/

      image.close(); //no longer need its memory in sysmem now that it's in GPU
      glBindTexture(GL_TEXTURE_2D,0); //unbind
    }
    checkError();
  }

  void activate(){
    //glActiveTexture(GL_TEXTURE0+textureIdx);
    glBindTexture(GL_TEXTURE_2D,textureBinding);
    checkError();
  }
};
int Texture::textureCount = 0;
Texture brick1,brick2;

struct GameState{
  bool paused;
  GameState(bool paused){
    this->paused = paused;
  }
};
GameState curState = GameState(true);

void drawBoard(){
  //glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
    glTexCoord2d(0,0);glVertex2d(-1,-1);
    glTexCoord2d(1,0);glVertex2d(1,-1);
    glTexCoord2d(1,1);glVertex2d(1,1);
    glTexCoord2d(0,1);glVertex2d(-1,1);
  glEnd();
  //glDisable(GL_TEXTURE_2D);
}
void drawLoot(){}
void drawMonsters(){}
void drawPlayer(){}
void drawGUI(){}

void draw(void) {
  glClear(GL_COLOR_BUFFER_BIT);

  //if(!curState.paused){
    drawBoard();
    drawLoot();
    drawMonsters();
    drawPlayer();
  //}else drawGUI();

  glutSwapBuffers();
}

bool fullscreen = false;
int resolution[2] = {1024,1024};
#define ESC 27
void keyPressed(unsigned char key, int mouseX, int mouseY){
  switch(key){
    case 'f':
      fullscreen = !fullscreen;
      if(fullscreen){
        printf("fullscreening...\n");
        glutFullScreen();
      }else{
        printf("undoing fullscreen.\n");
        glutReshapeWindow(resolution[0],resolution[1]);
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
      if(curState.paused) brick1.activate();
      else brick2.activate();
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
}


//Main program
//typedef unsigned int uint;
int main(int argc, char **argv) {
  glutInit(&argc, argv); 
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

  
  //glutInitWindowPosition(50, 25);
  glutInitWindowSize(resolution[0],resolution[1]);
  glutCreateWindow("Dungeon");
  
  //set Background color
  glEnable(GL_BLEND);
  glEnable(GL_ALPHA_TEST);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_TEXTURE_2D);
  glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0, 0, 0, 1);

  //setup matrices??
  //glOrtho(0,resolution[0],resolution[1],0,-1,1);

  //load images
  brick1 = Texture((char*)"textures/brick.jpg");
  brick2 = Texture((char*)"textures/brick.png");

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
