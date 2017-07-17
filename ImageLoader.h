struct Image{
  int width,height,numComponents;
  unsigned char* data;
  void close();
};

Image readImage(char* filename);

//Private functions in file:
//Image readJPEG(char* filename);
//Image readPNG(char* filename);
