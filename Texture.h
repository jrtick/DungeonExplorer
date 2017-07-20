struct Point{
  float x,y;
  Point(){}
  Point(float x, float y){
    this->x = x;
    this->y = y;
  }
  Point operator+(Point p){return Point(x+p.x,y+p.y);}
  Point operator-(Point p){return Point(x-p.x,y-p.y);}
  Point operator*(Point p){return Point(x*p.x,y*p.y);}
  Point operator+(float c){return Point(x + c,y + c);}
  Point operator-(float c){return Point(x - c,y - c);}
  Point operator*(float c){return Point(x * c,y * c);}
  void operator+=(Point p){
    this->x += p.x;
    this->y += p.y;
  }
  void operator-=(Point p){
    this->x -= p.x;
    this->y -= p.y;
  }
  void operator*=(Point p){
    this->x *= p.x;
    this->y *= p.y;
  }
  void operator+=(float x){
    this->x += x;
    this->y += x;
  }
  void operator-=(float x){
    this->x -= x;
    this->y -= x;
  }
  void operator*=(float x){
    this->x *= x;
    this->y *= y;
  }
};

struct Animation{
  int* indices;
  int indexCount;
  float speed;
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
  Animation* animations = (Animation*) NULL;
  int animationCount = 0, maxAnimations = 0;
  int activeAnimation = -1;
  float activeAnimationIdx = -1;
  SpriteSheet() : Texture(){}
  SpriteSheet(char* filename, int numRows, int numCols) : Texture(filename){
    this->numRows = numRows;
    this->numCols = numCols;
  }
  void drawQuad(Point p1, Point p2, Point p3, Point p4, int idx=0);
  int addAnimation(int* animationIndices, int count, int speed=1){
    if(animationCount+1>maxAnimations){ //need to resize
      maxAnimations += 5;
      Animation* newptr = (Animation*) malloc(sizeof(Animation)*maxAnimations);
      for(int i=0;i<animationCount;i++) newptr[i] = animations[i];
      if(animations) free(animations);
      animations = newptr;
    }
    animations[animationCount].indices = (int*) malloc(sizeof(int)*count);
    for(int i=0;i<count;i++)
      animations[animationCount].indices[i] = animationIndices[i];
    animations[animationCount].indexCount = count;
    animations[animationCount].speed = 1.f/float(speed);
    animationCount++;
    return animationCount-1;
  }
  void setAnimation(int animationIdx){
    if(animationIdx < animationCount) activeAnimation = animationIdx;
    activeAnimationIdx = 0;
  }
  void draw(Point p1, Point p2, Point p3, Point p4){
    Animation* anim = &animations[activeAnimation];
    drawQuad(p1,p2,p3,p4,anim->indices[int(activeAnimationIdx)]);
    activeAnimationIdx += anim->speed;
    if(activeAnimationIdx > anim->indexCount) activeAnimationIdx = 0;
  }
};
