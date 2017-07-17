#include <GL/glut.h>
#include <stdio.h>
#include "ImageLoader.h"


void draw(void) {
  glClear(GL_COLOR_BUFFER_BIT);

  //draw textures first
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
    glTexCoord2d(0,0);glVertex2d(-1,-1);
    glTexCoord2d(1,0);glVertex2d(1,-1);
    glTexCoord2d(1,1);glVertex2d(1,1);
    glTexCoord2d(0,1);glVertex2d(-1,1);
  glEnd();
  glDisable(GL_TEXTURE_2D);

  //draw nontextures
  glBegin(GL_QUADS);
    glVertex2d(-0.5,-0.5);
    glVertex2d(0.5,-0.5);
    glVertex2d(0.5,0.5);
    glVertex2d(-0.5,0.5);
  glEnd();

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
      printf("Exiting program...\n");
      exit(0);
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
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //when scaling down
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //when scaling up

  const char* filename = "textures/brick.jpg";
  Image image = readImage((char*)filename);
  if(image.numComponents < 0){
    printf("Image load failed\n");
    return -1;
  }
  printf("image dimensions = (%d,%d,%d)\n",image.width,image.height,image.numComponents);

  //create texture
  GLuint brickTexture;
  glGenTextures(1,&brickTexture);
  glBindTexture(GL_TEXTURE_2D, brickTexture);
  gluBuild2DMipmaps(GL_TEXTURE_2D,(image.numComponents==3)? GL_RGB8 : GL_RGBA8,image.width,image.height,(image.numComponents==3)? GL_RGB : GL_RGBA,GL_UNSIGNED_BYTE,image.data);
  image.close(); //no longer need its memory in sysmem 

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
