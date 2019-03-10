#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef struct {
    int colored, width, height, max_value;
    unsigned char** pixels; 
}image;

float identityFilter[3][3] = {{0, 0, 0}, {0, 1, 0}, {0, 0, 0}};
float smoothingFilter[3][3] = {{(float)1/9, (float)1/9, (float)1/9}, {(float)1/9, (float)1/9, (float)1/9}, {(float)1/9, (float)1/9, (float)1/9}};
float gaussianBlurFilter[3][3] = {{(float)1/16, (float)2/16, (float)1/16}, {(float)2/16, (float)4/16, (float)2/16}, {(float)1/16, (float)2/16, (float)1/16}};
float sharpenFilter[3][3] = {{(float)0/3, (float)-2/3, (float)0/3}, {(float)-2/3, (float)11/3, (float)-2/3}, {(float)0/3, (float)-2/3, (float)0/3}};
float meanRemovalFilter[3][3] = {{-1, -1, -1}, {-1, 9, -1}, {-1, -1, -1}};
float embossFilter[3][3] = {{0, 1, 0}, {0, 0, 0}, {0, -1, 0}};

image in, out;
image *aux_in;
int rank;
int nProcesses;

unsigned char** allocMatrix(int n, int m) {

	unsigned char** matrix;

	matrix = (unsigned char**) malloc(sizeof(unsigned char *) * n);
	if(matrix == NULL) {
		exit(1);
	}

	int i;

	for(i = 0; i < n; i++) {
		matrix[i] = (unsigned char*) malloc(sizeof(unsigned char) * m);
		if(matrix[i] == NULL) {
			exit(1);
		}
	}

	return(matrix);
}

void readInput(const char * fileName, image *img) {

    FILE *filePointer;

    char line[50];
    char *token;
    int linepixelnum;

    filePointer = fopen(fileName, "rb");
    if(filePointer == NULL)
        exit(1);
    if(fgets(line, sizeof(line), filePointer) == NULL)
        exit(1);
    if(line[1] - '0' == 5)
        img -> colored = 0;
    if(line[1] - '0' == 6)
        img -> colored = 1;

    if(fgets(line, sizeof(line), filePointer) == NULL)
        exit(1);
    if( (token = strtok(line, " \n")) != NULL ) {
        img -> width = atoi(token);
        token = strtok(NULL, " \n");
        img -> height = atoi(token);
    }

    do{
        if(fgets(line, sizeof(line), filePointer) == NULL)
            exit(1);
    }while(!isdigit(line[0]));
        img -> max_value = atoi(line); 

    if(img -> colored){
        linepixelnum = img -> width * 3;
        img -> pixels = allocMatrix(img -> height, linepixelnum);
        for(int i = 0; i < img -> height; i++){
            fread(img -> pixels[i], sizeof(unsigned char), linepixelnum, filePointer);
        }

    } else {
        linepixelnum = img -> width;
        img -> pixels = allocMatrix(img -> height, linepixelnum);
        for(int i = 0; i < img -> height; i++){
            fread(img -> pixels[i], sizeof(unsigned char), linepixelnum, filePointer);
        }
    }
    
    fclose(filePointer);
}

void writeData(const char * fileName, image *img) {

    FILE *filePointer;
    int linepixelnum;
    
    filePointer = fopen(fileName, "wb");
    if(filePointer == NULL)
        exit(1);

    if(img -> colored){
        linepixelnum = img -> width * 3;
        
        fprintf(filePointer, "P6\n");
        fprintf(filePointer, "%d %d\n", img -> width, img -> height);
        fprintf(filePointer, "%d\n", img -> max_value);

        for(int i = 0; i <  img -> height; i++){
            fwrite(img -> pixels[i], sizeof(unsigned char), linepixelnum, filePointer);
        }

    } else {
        linepixelnum = img -> width;

        fprintf(filePointer, "P5\n");
        fprintf(filePointer, "%d %d\n", img -> width, img -> height);
        fprintf(filePointer, "%d\n", img -> max_value);

        for(int i = 0; i < img -> height; i++){
            fwrite(img -> pixels[i], sizeof(unsigned char), linepixelnum, filePointer);
        }
    }

    fclose(filePointer);
}

void applyFilter(image *out, const char *filter){

    float sum;
    float red, green, blue;
    int start = rank * ceil((double)aux_in->height/nProcesses);
	int end = MIN(aux_in->height, (rank + 1) * ceil((double)aux_in->height/nProcesses));
    if(aux_in -> colored == 1){
        for(int i = start; i < end; i++){
            for(int j = 0; j < 3 * aux_in -> width; j += 3)
                if(i != 0 && i != (aux_in -> height - 1) && j != 0 && j != 1 && j != 2 && j != (3 * aux_in -> width - 1) && j != (3 * aux_in -> width - 2) && j != (3 * aux_in -> width - 3)){
                    if(strcmp(filter, "blur") == 0){
                        red = aux_in->pixels[i - 1][j - 3] * gaussianBlurFilter[0][0] +
                            aux_in->pixels[i - 1][j] * gaussianBlurFilter[0][1] +
                            aux_in->pixels[i - 1][j + 3] * gaussianBlurFilter[0][2] +
                            aux_in->pixels[i][j - 3] * gaussianBlurFilter[1][0] +
                            aux_in->pixels[i][j] * gaussianBlurFilter[1][1] +
                            aux_in->pixels[i][j + 3] * gaussianBlurFilter[1][2] +
                            aux_in->pixels[i + 1][j - 3] * gaussianBlurFilter[2][0] +
                            aux_in->pixels[i + 1][j] * gaussianBlurFilter[2][1] +
                            aux_in->pixels[i + 1][j + 3] * gaussianBlurFilter[2][2];
                        green = aux_in->pixels[i - 1][j - 2] * gaussianBlurFilter[0][0] +
                            aux_in->pixels[i - 1][j + 1] * gaussianBlurFilter[0][1] +
                            aux_in->pixels[i - 1][j + 4] * gaussianBlurFilter[0][2] +
                            aux_in->pixels[i][j - 2] * gaussianBlurFilter[1][0] +
                            aux_in->pixels[i][j + 1] * gaussianBlurFilter[1][1] +
                            aux_in->pixels[i][j + 4] * gaussianBlurFilter[1][2] +
                            aux_in->pixels[i + 1][j - 2] * gaussianBlurFilter[2][0] +
                            aux_in->pixels[i + 1][j + 1] * gaussianBlurFilter[2][1] +
                            aux_in->pixels[i + 1][j + 4] * gaussianBlurFilter[2][2];
                        blue = aux_in->pixels[i - 1][j - 1] * gaussianBlurFilter[0][0] +
                            aux_in->pixels[i - 1][j + 2] * gaussianBlurFilter[0][1] +
                            aux_in->pixels[i - 1][j + 5] * gaussianBlurFilter[0][2] +
                            aux_in->pixels[i][j - 1] * gaussianBlurFilter[1][0] +
                            aux_in->pixels[i][j + 2] * gaussianBlurFilter[1][1] +
                            aux_in->pixels[i][j + 5] * gaussianBlurFilter[1][2] +
                            aux_in->pixels[i + 1][j - 1] * gaussianBlurFilter[2][0] +
                            aux_in->pixels[i + 1][j + 2] * gaussianBlurFilter[2][1] +
                            aux_in->pixels[i + 1][j + 5] * gaussianBlurFilter[2][2];

                        out->pixels[i][j] = red;
                        out->pixels[i][j + 1] = green;
                        out->pixels[i][j + 2] = blue;
                    }
    
                    if(strcmp(filter, "smooth") == 0){
                        red = aux_in->pixels[i - 1][j - 3] * smoothingFilter[0][0] +
                            aux_in->pixels[i - 1][j] * smoothingFilter[0][1] +
                            aux_in->pixels[i - 1][j + 3] * smoothingFilter[0][2] +
                            aux_in->pixels[i][j - 3] * smoothingFilter[1][0] +
                            aux_in->pixels[i][j] * smoothingFilter[1][1] +
                            aux_in->pixels[i][j + 3] * smoothingFilter[1][2] +
                            aux_in->pixels[i + 1][j - 3] * smoothingFilter[2][0] +
                            aux_in->pixels[i + 1][j] * smoothingFilter[2][1] +
                            aux_in->pixels[i + 1][j + 3] * smoothingFilter[2][2];
                        green = aux_in->pixels[i - 1][j - 2] * smoothingFilter[0][0] +
                            aux_in->pixels[i - 1][j + 1] * smoothingFilter[0][1] +
                            aux_in->pixels[i - 1][j + 4] * smoothingFilter[0][2] +
                            aux_in->pixels[i][j - 2] * smoothingFilter[1][0] +
                            aux_in->pixels[i][j + 1] * smoothingFilter[1][1] +
                            aux_in->pixels[i][j + 4] * smoothingFilter[1][2] +
                            aux_in->pixels[i + 1][j - 2] * smoothingFilter[2][0] +
                            aux_in->pixels[i + 1][j + 1] * smoothingFilter[2][1] +
                            aux_in->pixels[i + 1][j + 4] * smoothingFilter[2][2];
                        blue = aux_in->pixels[i - 1][j - 1] * smoothingFilter[0][0] +
                            aux_in->pixels[i - 1][j + 2] * smoothingFilter[0][1] +
                            aux_in->pixels[i - 1][j + 5] * smoothingFilter[0][2] +
                            aux_in->pixels[i][j - 1] * smoothingFilter[1][0] +
                            aux_in->pixels[i][j + 2] * smoothingFilter[1][1] +
                            aux_in->pixels[i][j + 5] * smoothingFilter[1][2] +
                            aux_in->pixels[i + 1][j - 1] * smoothingFilter[2][0] +
                            aux_in->pixels[i + 1][j + 2] * smoothingFilter[2][1] +
                            aux_in->pixels[i + 1][j + 5] * smoothingFilter[2][2];

                        out->pixels[i][j] = red;
                        out->pixels[i][j + 1] = green;
                        out->pixels[i][j + 2] = blue;
                    }

                    if(strcmp(filter, "sharpen") == 0){
                        red = aux_in->pixels[i - 1][j - 3] * sharpenFilter[0][0] +
                            aux_in->pixels[i - 1][j] * sharpenFilter[0][1] +
                            aux_in->pixels[i - 1][j + 3] * sharpenFilter[0][2] +
                            aux_in->pixels[i][j - 3] * sharpenFilter[1][0] +
                            aux_in->pixels[i][j] * sharpenFilter[1][1] +
                            aux_in->pixels[i][j + 3] * sharpenFilter[1][2] +
                            aux_in->pixels[i + 1][j - 3] * sharpenFilter[2][0] +
                            aux_in->pixels[i + 1][j] * sharpenFilter[2][1] +
                            aux_in->pixels[i + 1][j + 3] * sharpenFilter[2][2];
                        green = aux_in->pixels[i - 1][j - 2] * sharpenFilter[0][0] +
                            aux_in->pixels[i - 1][j + 1] * sharpenFilter[0][1] +
                            aux_in->pixels[i - 1][j + 4] * sharpenFilter[0][2] +
                            aux_in->pixels[i][j - 2] * sharpenFilter[1][0] +
                            aux_in->pixels[i][j + 1] * sharpenFilter[1][1] +
                            aux_in->pixels[i][j + 4] * sharpenFilter[1][2] +
                            aux_in->pixels[i + 1][j - 2] * sharpenFilter[2][0] +
                            aux_in->pixels[i + 1][j + 1] * sharpenFilter[2][1] +
                            aux_in->pixels[i + 1][j + 4] * sharpenFilter[2][2];
                        blue = aux_in->pixels[i - 1][j - 1] * sharpenFilter[0][0] +
                            aux_in->pixels[i - 1][j + 2] * sharpenFilter[0][1] +
                            aux_in->pixels[i - 1][j + 5] * sharpenFilter[0][2] +
                            aux_in->pixels[i][j - 1] * sharpenFilter[1][0] +
                            aux_in->pixels[i][j + 2] * sharpenFilter[1][1] +
                            aux_in->pixels[i][j + 5] * sharpenFilter[1][2] +
                            aux_in->pixels[i + 1][j - 1] * sharpenFilter[2][0] +
                            aux_in->pixels[i + 1][j + 2] * sharpenFilter[2][1] +
                            aux_in->pixels[i + 1][j + 5] * sharpenFilter[2][2];

                        out->pixels[i][j] = red;
                        out->pixels[i][j + 1] = green;
                        out->pixels[i][j + 2] = blue;
                    }

                    if(strcmp(filter, "emboss") == 0){
                        red = aux_in->pixels[i - 1][j - 3] * embossFilter[0][0] +
                            aux_in->pixels[i - 1][j] * embossFilter[0][1] +
                            aux_in->pixels[i - 1][j + 3] * embossFilter[0][2] +
                            aux_in->pixels[i][j - 3] * embossFilter[1][0] +
                            aux_in->pixels[i][j] * embossFilter[1][1] +
                            aux_in->pixels[i][j + 3] * embossFilter[1][2] +
                            aux_in->pixels[i + 1][j - 3] * embossFilter[2][0] +
                            aux_in->pixels[i + 1][j] * embossFilter[2][1] +
                            aux_in->pixels[i + 1][j + 3] * embossFilter[2][2];
                        green = aux_in->pixels[i - 1][j - 2] * embossFilter[0][0] +
                            aux_in->pixels[i - 1][j + 1] * embossFilter[0][1] +
                            aux_in->pixels[i - 1][j + 4] * embossFilter[0][2] +
                            aux_in->pixels[i][j - 2] * embossFilter[1][0] +
                            aux_in->pixels[i][j + 1] * embossFilter[1][1] +
                            aux_in->pixels[i][j + 4] * embossFilter[1][2] +
                            aux_in->pixels[i + 1][j - 2] * embossFilter[2][0] +
                            aux_in->pixels[i + 1][j + 1] * embossFilter[2][1] +
                            aux_in->pixels[i + 1][j + 4] * embossFilter[2][2];
                        blue = aux_in->pixels[i - 1][j - 1] * embossFilter[0][0] +
                            aux_in->pixels[i - 1][j + 2] * embossFilter[0][1] +
                            aux_in->pixels[i - 1][j + 5] * embossFilter[0][2] +
                            aux_in->pixels[i][j - 1] * embossFilter[1][0] +
                            aux_in->pixels[i][j + 2] * embossFilter[1][1] +
                            aux_in->pixels[i][j + 5] * embossFilter[1][2] +
                            aux_in->pixels[i + 1][j - 1] * embossFilter[2][0] +
                            aux_in->pixels[i + 1][j + 2] * embossFilter[2][1] +
                            aux_in->pixels[i + 1][j + 5] * embossFilter[2][2];

                        out->pixels[i][j] = red;
                        out->pixels[i][j + 1] = green;
                        out->pixels[i][j + 2] = blue;
                    }

                    if(strcmp(filter, "mean") == 0){
                        red = aux_in->pixels[i - 1][j - 3] * meanRemovalFilter[0][0] +
                            aux_in->pixels[i - 1][j] * meanRemovalFilter[0][1] +
                            aux_in->pixels[i - 1][j + 3] * meanRemovalFilter[0][2] +
                            aux_in->pixels[i][j - 3] * meanRemovalFilter[1][0] +
                            aux_in->pixels[i][j] * meanRemovalFilter[1][1] +
                            aux_in->pixels[i][j + 3] * meanRemovalFilter[1][2] +
                            aux_in->pixels[i + 1][j - 3] * meanRemovalFilter[2][0] +
                            aux_in->pixels[i + 1][j] * meanRemovalFilter[2][1] +
                            aux_in->pixels[i + 1][j + 3] * meanRemovalFilter[2][2];
                        green = aux_in->pixels[i - 1][j - 2] * meanRemovalFilter[0][0] +
                            aux_in->pixels[i - 1][j + 1] * meanRemovalFilter[0][1] +
                            aux_in->pixels[i - 1][j + 4] * meanRemovalFilter[0][2] +
                            aux_in->pixels[i][j - 2] * meanRemovalFilter[1][0] +
                            aux_in->pixels[i][j + 1] * meanRemovalFilter[1][1] +
                            aux_in->pixels[i][j + 4] * meanRemovalFilter[1][2] +
                            aux_in->pixels[i + 1][j - 2] * meanRemovalFilter[2][0] +
                            aux_in->pixels[i + 1][j + 1] * meanRemovalFilter[2][1] +
                            aux_in->pixels[i + 1][j + 4] * meanRemovalFilter[2][2];
                        blue = aux_in->pixels[i - 1][j - 1] * meanRemovalFilter[0][0] +
                            aux_in->pixels[i - 1][j + 2] * meanRemovalFilter[0][1] +
                            aux_in->pixels[i - 1][j + 5] * meanRemovalFilter[0][2] +
                            aux_in->pixels[i][j - 1] * meanRemovalFilter[1][0] +
                            aux_in->pixels[i][j + 2] * meanRemovalFilter[1][1] +
                            aux_in->pixels[i][j + 5] * meanRemovalFilter[1][2] +
                            aux_in->pixels[i + 1][j - 1] * meanRemovalFilter[2][0] +
                            aux_in->pixels[i + 1][j + 2] * meanRemovalFilter[2][1] +
                            aux_in->pixels[i + 1][j + 5] * meanRemovalFilter[2][2];

                        out->pixels[i][j] = red;
                        out->pixels[i][j + 1] = green;
                        out->pixels[i][j + 2] = blue;
                    }

                    if(strcmp(filter, "id") == 0){
                        red = aux_in->pixels[i - 1][j - 3] * identityFilter[0][0] +
                            aux_in->pixels[i - 1][j] * identityFilter[0][1] +
                            aux_in->pixels[i - 1][j + 3] * identityFilter[0][2] +
                            aux_in->pixels[i][j - 3] * identityFilter[1][0] +
                            aux_in->pixels[i][j] * identityFilter[1][1] +
                            aux_in->pixels[i][j + 3] * identityFilter[1][2] +
                            aux_in->pixels[i + 1][j - 3] * identityFilter[2][0] +
                            aux_in->pixels[i + 1][j] * identityFilter[2][1] +
                            aux_in->pixels[i + 1][j + 3] * identityFilter[2][2];
                        green = aux_in->pixels[i - 1][j - 2] * identityFilter[0][0] +
                            aux_in->pixels[i - 1][j + 1] * identityFilter[0][1] +
                            aux_in->pixels[i - 1][j + 4] * identityFilter[0][2] +
                            aux_in->pixels[i][j - 2] * identityFilter[1][0] +
                            aux_in->pixels[i][j + 1] * identityFilter[1][1] +
                            aux_in->pixels[i][j + 4] * identityFilter[1][2] +
                            aux_in->pixels[i + 1][j - 2] * identityFilter[2][0] +
                            aux_in->pixels[i + 1][j + 1] * identityFilter[2][1] +
                            aux_in->pixels[i + 1][j + 4] * identityFilter[2][2];
                        blue = aux_in->pixels[i - 1][j - 1] * identityFilter[0][0] +
                            aux_in->pixels[i - 1][j + 2] * identityFilter[0][1] +
                            aux_in->pixels[i - 1][j + 5] * identityFilter[0][2] +
                            aux_in->pixels[i][j - 1] * identityFilter[1][0] +
                            aux_in->pixels[i][j + 2] * identityFilter[1][1] +
                            aux_in->pixels[i][j + 5] * identityFilter[1][2] +
                            aux_in->pixels[i + 1][j - 1] * identityFilter[2][0] +
                            aux_in->pixels[i + 1][j + 2] * identityFilter[2][1] +
                            aux_in->pixels[i + 1][j + 5] * identityFilter[2][2];

                        out->pixels[i][j] = red;
                        out->pixels[i][j + 1] = green;
                        out->pixels[i][j + 2] = blue;
                    }    
                }
        }
    } else {
        for(int i = start; i < end; i++){
            for(int j = 0; j < aux_in -> width; j++)
            {
                if(i != 0 && i != (aux_in -> height - 1) && j != 0 && j != (aux_in -> width - 1)){
                    if(strcmp(filter, "blur") == 0){
                        sum = aux_in->pixels[i - 1][j - 1] * gaussianBlurFilter[0][0] + 
                            aux_in->pixels[i - 1][j] * gaussianBlurFilter[0][1] + 
                            aux_in->pixels[i - 1][j + 1] * gaussianBlurFilter[0][2] +
                            aux_in->pixels[i][j - 1] * gaussianBlurFilter[1][0] + 
                            aux_in->pixels[i][j] * gaussianBlurFilter[1][1] + 
                            aux_in->pixels[i][j + 1] * gaussianBlurFilter[1][2] + 
                            aux_in->pixels[i + 1][j - 1] * gaussianBlurFilter[2][0] + 
                            aux_in->pixels[i + 1][j] * gaussianBlurFilter[2][1] + 
                            aux_in->pixels[i + 1][j + 1] * gaussianBlurFilter[2][2];
                        
                        out->pixels[i][j] = sum;
                    }

                    if(strcmp(filter, "smooth") == 0){
                        sum = aux_in->pixels[i - 1][j - 1] * smoothingFilter[0][0] + 
                            aux_in->pixels[i - 1][j] * smoothingFilter[0][1] + 
                            aux_in->pixels[i - 1][j + 1] * smoothingFilter[0][2] +
                            aux_in->pixels[i][j - 1] * smoothingFilter[1][0] + 
                            aux_in->pixels[i][j] * smoothingFilter[1][1] + 
                            aux_in->pixels[i][j + 1] * smoothingFilter[1][2] + 
                            aux_in->pixels[i + 1][j - 1] * smoothingFilter[2][0] + 
                            aux_in->pixels[i + 1][j] * smoothingFilter[2][1] + 
                            aux_in->pixels[i + 1][j + 1] * smoothingFilter[2][2];
                        out->pixels[i][j] = sum;
                    }

                    if(strcmp(filter, "sharpen") == 0){
                        sum = aux_in->pixels[i - 1][j - 1] * sharpenFilter[0][0] + 
                            aux_in->pixels[i - 1][j] * sharpenFilter[0][1] + 
                            aux_in->pixels[i - 1][j + 1] * sharpenFilter[0][2] +
                            aux_in->pixels[i][j - 1] * sharpenFilter[1][0] + 
                            aux_in->pixels[i][j] * sharpenFilter[1][1] + 
                            aux_in->pixels[i][j + 1] * sharpenFilter[1][2] + 
                            aux_in->pixels[i + 1][j - 1] * sharpenFilter[2][0] + 
                            aux_in->pixels[i + 1][j] * sharpenFilter[2][1] + 
                            aux_in->pixels[i + 1][j + 1] * sharpenFilter[2][2];
                        out->pixels[i][j] = sum;
                    }

                    if(strcmp(filter, "emboss") == 0){
                        sum = aux_in->pixels[i - 1][j - 1] * embossFilter[0][0] + 
                            aux_in->pixels[i - 1][j] * embossFilter[0][1] + 
                            aux_in->pixels[i - 1][j + 1] * embossFilter[0][2] +
                            aux_in->pixels[i][j - 1] * embossFilter[1][0] + 
                            aux_in->pixels[i][j] * embossFilter[1][1] + 
                            aux_in->pixels[i][j + 1] * embossFilter[1][2] + 
                            aux_in->pixels[i + 1][j - 1] * embossFilter[2][0] + 
                            aux_in->pixels[i + 1][j] * embossFilter[2][1] + 
                            aux_in->pixels[i + 1][j + 1] * embossFilter[2][2];
                        out->pixels[i][j] = sum;
                    }

                    if(strcmp(filter, "mean") == 0){
                        sum = aux_in->pixels[i - 1][j - 1] * meanRemovalFilter[0][0] + 
                            aux_in->pixels[i - 1][j] * meanRemovalFilter[0][1] + 
                            aux_in->pixels[i - 1][j + 1] * meanRemovalFilter[0][2] +
                            aux_in->pixels[i][j - 1] * meanRemovalFilter[1][0] + 
                            aux_in->pixels[i][j] * meanRemovalFilter[1][1] + 
                            aux_in->pixels[i][j + 1] * meanRemovalFilter[1][2] + 
                            aux_in->pixels[i + 1][j - 1] * meanRemovalFilter[2][0] + 
                            aux_in->pixels[i + 1][j] * meanRemovalFilter[2][1] + 
                            aux_in->pixels[i + 1][j + 1] * meanRemovalFilter[2][2];
                        out->pixels[i][j] = sum;
                    }

                    if(strcmp(filter, "id") == 0){
                        sum = aux_in->pixels[i - 1][j - 1] * identityFilter[0][0] + 
                            aux_in->pixels[i - 1][j] * identityFilter[0][1] + 
                            aux_in->pixels[i - 1][j + 1] * identityFilter[0][2] +
                            aux_in->pixels[i][j - 1] * identityFilter[1][0] + 
                            aux_in->pixels[i][j] * identityFilter[1][1] + 
                            aux_in->pixels[i][j + 1] * identityFilter[1][2] + 
                            aux_in->pixels[i + 1][j - 1] * identityFilter[2][0] + 
                            aux_in->pixels[i + 1][j] * identityFilter[2][1] + 
                            aux_in->pixels[i + 1][j + 1] * identityFilter[2][2];
                        out->pixels[i][j] = sum;
                    }    
                }
            }
        }
    }
}


int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    const int root = 0;
    unsigned char *pixelArray;
    

    const char *filter;
    if(argc < 3) {
		exit(-1);
	}

    if(rank == root)
        readInput(argv[1], &in);

    MPI_Bcast(&in.colored, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&in.width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&in.height, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&in.max_value, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(in.colored == 0){
        pixelArray = (unsigned char*) malloc(sizeof(unsigned char) * in.width * in.height);
    }
    else {
        pixelArray = (unsigned char*) malloc(sizeof(unsigned char) * in.width * in.height * 3);
    }

    if(rank == root){
        if(in.colored == 0){
            for(int i = 0; i < in.height; i++)
                for(int j = 0; j < in.width; j++)
                    pixelArray[i * in.width + j] = in.pixels[i][j];
        }
        else {
            for(int i = 0; i < in.height; i++)
                for(int j = 0; j < 3 * in.width; j++)
                    pixelArray[i * (3 * in.width) + j] = in.pixels[i][j];
        }
    }else {
        if(in.colored == 0)
            in.pixels = allocMatrix(in.height, in.width);
        else
            in.pixels = allocMatrix(in.height, 3 * in.width);
    }

    if(in.colored == 0)
        MPI_Bcast(pixelArray, in.width * in.height, MPI_UNSIGNED_CHAR, root, MPI_COMM_WORLD);
    else
        MPI_Bcast(pixelArray, 3 * in.width * in.height, MPI_UNSIGNED_CHAR, root, MPI_COMM_WORLD);

    if(rank != root){
        if(in.colored == 0){
            for(int i = 0; i < in.height; i++)
                for(int j = 0; j < in.width; j++)
                    in.pixels[i][j] = pixelArray[i * in.width + j];
                    
        }
        else {
            for(int i = 0; i < in.height; i++)
                for(int j = 0; j < 3 * in.width; j++)
                    in.pixels[i][j] = pixelArray[i * (3 * in.width) + j];
        }
    }

    aux_in = &in;

    out.colored = aux_in -> colored;
    out.max_value = aux_in -> max_value; 
    out.width = aux_in -> width;
    out.height = aux_in -> height;
    
    if(aux_in -> colored == 0){
        out.pixels = allocMatrix( out.height, out.width);
        for(int i = 0; i < aux_in -> height; i++)
            for(int j = 0; j < aux_in -> width; j++)
                out.pixels[i][j] = aux_in -> pixels[i][j];
    }
    else {
        out.pixels = allocMatrix( out.height, 3 * out.width);
        for(int i = 0; i < aux_in -> height; i++)
            for(int j = 0; j < 3 * aux_in -> width; j++)
                out.pixels[i][j] = aux_in -> pixels[i][j];
    }

    if(argc > 3){
        int start, end;
        for(int i = 3; i < argc; i++){
            filter = argv[i];
            applyFilter(&out, filter);
            if(rank != root){
                start = rank * ceil((double)aux_in->height/nProcesses);
	            end = MIN(aux_in->height, (rank + 1) * ceil((double)aux_in->height/nProcesses));

                if(in.colored == 0){
                    for(int i = start; i < end; i++)
                        for(int j = 0; j < in.width; j++)
                            pixelArray[i * in.width + j] = out.pixels[i][j];
                }
                else {
                    for(int i = start; i < end; i++)
                        for(int j = 0; j < 3 * in.width; j++)
                            pixelArray[i * (3 * in.width) + j] = out.pixels[i][j];
                }
     
                if(in.colored == 0){
                    MPI_Send(pixelArray + (in.width * start), (end - start) * in.width, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
                }
                else{
                    MPI_Send(pixelArray + (3 * out.width * start), (end - start) * (3*out.width), MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
                }
            }

            if(rank == root){
                start = rank * ceil((double)aux_in->height/nProcesses);
	            end = MIN(aux_in->height, (rank + 1) * ceil((double)aux_in->height/nProcesses)); 
                if(in.colored == 0){
                    for(int i = start; i < end; i++)
                        for(int j = 0; j < in.width; j++)
                            pixelArray[i * in.width + j] = out.pixels[i][j];
                }
                else {
                    for(int i = start; i < end; i++)
                        for(int j = 0; j < 3 * in.width; j++)
                            pixelArray[i * (3 * in.width) + j] = out.pixels[i][j];
                }
                

                for(int i = 1; i < nProcesses; i++){
                    start = i * ceil((double)aux_in->height/nProcesses);
	                end = MIN(aux_in->height, (i + 1) * ceil((double)aux_in->height/nProcesses));
                    if(in.colored == 0)
                        MPI_Recv(pixelArray + (out.width * start), (end - start) * out.width, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    else
                        MPI_Recv(pixelArray + (3 * out.width * start), (end - start) * (3*out.width), MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }

                if(in.colored == 0){
                    for(int i = 0; i < in.height; i++)
                        for(int j = 0; j < in.width; j++){
                            out.pixels[i][j] = pixelArray[i * in.width + j];
                            in.pixels[i][j] = pixelArray[i * in.width + j];
                        }
                            
                }
                else {
                    for(int i = 0; i < in.height; i++)
                        for(int j = 0; j < 3 * in.width; j++){
                            out.pixels[i][j] = pixelArray[i * (3 * in.width) + j];
                            in.pixels[i][j] = pixelArray[i * (3 * in.width) + j];
                        }
                }     
            }

            if(aux_in -> colored == 0)
                MPI_Bcast(pixelArray, in.width * in.height, MPI_UNSIGNED_CHAR, root, MPI_COMM_WORLD);
            else
                MPI_Bcast(pixelArray, 3 * in.width * in.height, MPI_UNSIGNED_CHAR, root, MPI_COMM_WORLD);

            if(rank != root){
                if(in.colored == 0){
                    for(int i = 0; i < in.height; i++)
                        for(int j = 0; j < in.width; j++)
                            in.pixels[i][j] = pixelArray[i * in.width + j];
                            
                }
                else {
                    for(int i = 0; i < in.height; i++)
                        for(int j = 0; j < 3 * in.width; j++)
                            in.pixels[i][j] = pixelArray[i * (3 * in.width) + j];
                }
            }

        }
    }

    if(rank == 0)
        writeData(argv[2], &out);
    
    MPI_Finalize();
    return 0;
}