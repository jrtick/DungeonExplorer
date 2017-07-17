#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <jpeglib.h>
#include <jerror.h>

#include <png.h>

//could apparently use "SOIL" (simple opengl image library) ??

#include "ImageLoader.h"
#define RETURN_ERROR(data) { data.numComponents = -1; return data; }


void Image::close(){
  if(data) free(data);
}

/* based on
 * https://stackoverflow.com/questions/5616216/need-help-in-reading-jpeg-file-using-libjpeg
 */
Image readJPEG(char* filename){
  Image data;

  FILE* file = fopen(filename, "rb");
  if(!file) {
     fprintf(stderr, "Error: JPEG file %s doesn't exist.", filename);
     RETURN_ERROR(data);
  }

  struct jpeg_decompress_struct info; //jpeg info
  struct jpeg_error_mgr err; //the error handler
  info.err = jpeg_std_error(&err);     
  jpeg_create_decompress(&info); //fills info structure
  jpeg_stdio_src(&info, file);    
  jpeg_read_header(&info, TRUE);   // read jpeg file header
  jpeg_start_decompress(&info);    // decompress the file

  data.width = info.output_width;
  data.height = info.output_height;
  data.numComponents = info.num_components;

  data.data = (unsigned char *)malloc(data.width*data.height
                                      *data.numComponents*sizeof(char));
  while (info.output_scanline < info.output_height){
    unsigned char* rowptr = (unsigned char *)data.data +
            data.numComponents*info.output_width*info.output_scanline; 
    jpeg_read_scanlines(&info, &rowptr, 1);
  }
  jpeg_finish_decompress(&info);
  jpeg_destroy_decompress(&info);
  fclose(file);

  return data;
}

/* based on
 * http://zarb.org/~gc/html/libpng.html
 */
Image readPNG(char* filename){
  Image data;
  FILE* file = fopen(filename,"rb");
  if(!file){
    fprintf(stderr,"Error: PNG file %s doesn't exist.",filename);
    RETURN_ERROR(data);
  }

  char header[8];
  fread((png_bytep) header,1,8,file);
  if(png_sig_cmp((png_bytep) header,0,8)) RETURN_ERROR(data);
  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                                 NULL,NULL,NULL);
  if(!png_ptr) RETURN_ERROR(data);

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if(setjmp(png_jmpbuf(png_ptr))) RETURN_ERROR(data);

  png_init_io(png_ptr,file);
  png_set_sig_bytes(png_ptr,8);

  png_read_info(png_ptr,info_ptr);

  data.width  = png_get_image_width (png_ptr,info_ptr);
  data.height = png_get_image_height(png_ptr,info_ptr);

  //read file
  if(setjmp(png_jmpbuf(png_ptr))) RETURN_ERROR(data);

  png_bytep* row_pointers = (png_bytep*) malloc(sizeof(png_bytep)*data.height);
  for(int y=0;y<data.height;y++)
    row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

  png_read_image(png_ptr,row_pointers);
  fclose(file);

  if(png_get_color_type(png_ptr,info_ptr) == PNG_COLOR_TYPE_RGBA)
    data.numComponents = 4;
  else data.numComponents = 3;

  data.data = (unsigned char*) malloc(data.width*data.height
                                      *data.numComponents*sizeof(char));
  for(int y=0;y<data.height;y++){
    png_byte* row = row_pointers[y];
    for(int x=0;x<data.width;x++){
      png_byte* ptr = &(row[data.numComponents*x]);
      int idx = data.numComponents*(y*data.width+x);
      data.data[idx]   = ptr[0];
      data.data[idx+1] = ptr[1];
      data.data[idx+2] = ptr[2];
      if(data.numComponents==4) data.data[idx+3] = ptr[3];
    }
  }

  for(int y=0;y<data.height;y++) free(row_pointers[y]);
  free(row_pointers);

  return data;
}

Image readImage(char* filename){
  int length=strlen(filename);

  bool valid = false;
  int extnIdx = -1;
  for(int i=0;i<length;i++){
    if(filename[i] == '.'){
      if(valid){ //can't have more than one period
        valid = false;
        break;
      }else{
        valid = true;
        extnIdx = i;
      }
    }
  }
  if(valid) valid = (extnIdx != length-1); //extension can't be zero length
  
  Image data;
  if(!valid) RETURN_ERROR(data); //invalid filename

  int newlength = length-extnIdx-1;
  char* extension = (char*) malloc(sizeof(char)*newlength);
  for(int i=0;i<newlength;i++) extension[i] = filename[extnIdx+1+i];
  extension[newlength] = '\0';

  if(!strcasecmp(extension,"png")) return readPNG(filename);
  else if(!strcasecmp(extension,"jpg") ||
          !strcasecmp(extension,"jpeg")) return readJPEG(filename);
  else{
    fprintf(stderr,"Error: extension %s not recognized\n.",extension);
    RETURN_ERROR(data);
  }
}
