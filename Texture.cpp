#include <GL/freeglut.h>
#include <stdio.h>
#include "ImageLoader.h"
#include "Texture.h"

void checkError(){
  GLenum errCode = glGetError();
  if(errCode != GL_NO_ERROR){
    printf("Error: %s\n",(char*) gluErrorString(errCode));
  }
}

int Texture::textureCount = 0;

Texture::Texture(){
  textureBinding = -1;
}

Texture::Texture(char* filename){
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

void Texture::activate(){
  //glActiveTexture(GL_TEXTURE0+textureIdx);
  glBindTexture(GL_TEXTURE_2D,textureBinding);
  checkError();
}

void Texture::drawQuad(Point p1, Point p2, Point p3, Point p4, int numTiles){
  activate();
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
    glTexCoord2d(0,0);glVertex2d(p1.x,p1.y);
    glTexCoord2d(numTiles,0);glVertex2d(p2.x,p2.y);
    glTexCoord2d(numTiles,numTiles);glVertex2d(p3.x,p3.y);
    glTexCoord2d(0,numTiles);glVertex2d(p4.x,p4.y);
  glEnd();
  glDisable(GL_TEXTURE_2D);
}

void SpriteSheet::drawQuad(Point p1, Point p2, Point p3, Point p4, int idx){
  activate();
  int row = idx / numCols,
      col = idx % numCols;
  float invNumRows = 1.f / float(numRows),
        invNumCols = 1.f / float(numCols);
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
    glTexCoord2d(col*invNumCols,(row+1)*invNumRows);glVertex2d(p1.x,p1.y);
    glTexCoord2d((col+1)*invNumCols,(row+1)*invNumRows);glVertex2d(p2.x,p2.y);
    glTexCoord2d((col+1)*invNumCols,row*invNumRows);glVertex2d(p3.x,p3.y);
    glTexCoord2d(col*invNumCols,row*invNumRows);glVertex2d(p4.x,p4.y);
  glEnd();
  glDisable(GL_TEXTURE_2D);
}
