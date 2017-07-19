struct Point{
  float x,y;
  Point(){}
  Point(float x, float y){
    this->x = x;
    this->y = y;
  }
};

class Texture{
public:
  static int textureCount;
  int width,height,textureIdx,channels;

  Texture();
  Texture(char* filename);
  void drawQuad(Point p1, Point p2, Point p3, Point p4, int numTiles=1);
  void activate();
protected:
  GLuint textureBinding; //if I make this private then the include file doesn't need glut.h
};

class SpriteSheet : public Texture {
public:
  int numRows, numCols;
  SpriteSheet() : Texture(){}
  SpriteSheet(char* filename, int numRows, int numCols) : Texture(filename){
    this->numRows = numRows;
    this->numCols = numCols;
  }
  void drawQuad(Point p1, Point p2, Point p3, Point p4, int idx=0);
};
