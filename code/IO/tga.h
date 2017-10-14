// TGA header

#ifndef TGAH
#define TGAH

enum 
{                            
TGA_ERROR_FILE_OPEN, 
TGA_ERROR_READING_FILE, 
TGA_ERROR_INDEXED_COLOR,
TGA_ERROR_MEMORY, 
TGA_ERROR_COMPRESSED_FILE,
TGA_OK 
};

int tgaSaveSeries(char *filename, short int width, short int height, unsigned char pixelDepth, unsigned char *imageData);

int tgaSave(    const char      *filename, 
                short int       width, 
                short int       height, 
                unsigned char   pixelDepth,
                unsigned char   *imageData);

typedef struct
{
	int status;
	unsigned char type, pixelDepth;
	short int width, height;
	unsigned char *imageData;
}
tgaInfo;

#endif

