#include "bmp8.h"


//debugging
void bmp8_compare_da_images() {

    t_bmp8 *img = (t_bmp8*) malloc(sizeof(t_bmp8));
    img = bmp8_loadImage("output_grayscale_8bit.bmp");


    t_bmp8 *b = (t_bmp8*) malloc(sizeof(t_bmp8));
    b = bmp8_loadImage("bright.bmp");

    bmp8_printInfo(img, 0);
    bmp8_printInfo(b, 0);


    //compare the header, element by element
    for (int i = 0; i < 54; i++) {

        if (img->header[i] != b->header[i]) {
            printf("or: %02x,  cp: %02x @ %d\n",img->header[i], b->header[i], i);
        }

    }


    //compare the color table, element by element
    for (int i = 0; i < 1024; i++) {

        if (img->colorTable[i] != b->colorTable[i]) {
            printf("or: %02x,  cp: %02x @ %d\n",img->colorTable[i], b->colorTable[i], i);
        }

    }

    //compare the width, height & colorDepth
    if (img->width != b->width){ printf("diff of width\n"); return; }
    if (img->height != b->height) {printf("diff of height\n"); return; }
    if (img->colorDepth != b->colorDepth) printf("diff of color depth\n");


    //compare the data
    for (int i = 0; i < img->width * img->height ; i++) {

        if (img->data[i] != b->data[i]) {
            printf("or: %02x,  cp: %02x @ %d\n",img->data[i], b->data[i], i);
        }

    }





    bmp8_free(b);
    bmp8_free(img);
}



//utils
t_bmp8 * bmp8_loadImage(const char * filename) {

    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf( "error opening file %s\n", filename);
        return NULL;
    }

    t_bmp8* bmp8 = (t_bmp8*)malloc(sizeof(t_bmp8));

    // lecture du header de l'image
    fread(bmp8->header, sizeof(unsigned char), 54, file);

    // extraction des info utile, en utilisant les offset qui sont donnés dans le sujet
    // c est un peu sorcier les lignes en dessous mais :
    // comme vu dans le sujet les info prennent une certain place ( 4bytes pour width et height, 2 pour color planes)
    // or le header il stockes les bytes 1 par 1, donc si on lit juste header[18] on loupe 3 octets.
    // La memoire dans un tableau de unsigned char elle ressemble a ca : [][][][][][]....[] avec chaque case stocké sur un byte
    // On sait qu'un int est stocké sur 4 bytes, donc on caste la lecture de header[18] en (int*) (pointeur car on manipule de la mémoire)
    // enfin on mets *(int*) pour avoir la valeur sur laquelle (int*) pointe.
    bmp8->width  = *(int*)&bmp8->header[18];

    // on pourrait aussi le lire de la sorte, en gros on des opérations bitwise.
    // premier bytes OU deuxieme bytes shifté 8 fois a gauche OU 3eme bytes shifté a gauche 16 fois OU 4eme bytes shifté a gauche 24 fois. On shift dans ce sens car on est en litle  endian
    int width = bmp8->header[18] | bmp8->header[19] << 8 | bmp8->header[20] << 16 | bmp8->header[21] << 24;

    bmp8->height = *(int*)&bmp8->header[22];
    bmp8->colorDepth = *(short*)&bmp8->header[28];

    // si y a plus ou moins de 256 couleurs ( 8bits ), on quitte le jeux
    if (bmp8->colorDepth != 8) {
        printf("error color plane (%d bits)\n", bmp8->colorDepth);
        fclose(file);
        return NULL;
    }

    // Dans le fichier bmp y a une liste de valeur pour chaque pixel, mais c est pas directement une couleur, c est un index.
    // dans le header on a la colorTable. On retrouve les valeurs RGB du pixel en allant a colorTable[index]
    fread(bmp8->colorTable, sizeof(unsigned char), 1024, file);


    // taille de la bitmap
    int rowSize = ((bmp8->width + 3) / 4) * 4; // pour respecter le padding
    bmp8->dataSize = rowSize * bmp8->height;

    // taille de data sur mesure, f(dataSize)
    bmp8->data = malloc(sizeof(unsigned char) * bmp8->dataSize);
    if (bmp8->data == NULL) {
        printf("error allocating memory\n");
        fclose(file);
        return NULL;
    }

    // Lecture des pixels
    fread(bmp8->data, sizeof(unsigned char), bmp8->dataSize, file);




    fclose(file);
    return bmp8;
}
void bmp8_saveImage(const char * filename, t_bmp8 * img){


  FILE* file = fopen(filename, "wb");
  if (file == NULL) {
    printf("error opening file %s\n", filename);
    return;
  }
  printf("Saving image to %s\n", filename);


  if(fwrite(img->header, sizeof(unsigned char), 54, file) != 54){
      printf("error saving image header\n");
      fclose(file);
      return;
  }
  if(fwrite(img->colorTable, sizeof(unsigned char), 1024, file) != 1024){
    printf("error saving image color table\n");
    fclose(file);
    return;
  }
  if(fwrite(img->data, sizeof(unsigned char), img->dataSize, file) != img->dataSize){
    printf("error saving image data\n");
    fclose(file);
    return;
  }

  printf("Image saved successfully\n");


  fclose(file);

}
void bmp8_free(t_bmp8 * img){
// j'avoue que la fonction me parait un peu débile, y a presque rien a free

  if (img){

      free(img->data);
      free(img);
      return;

  }
  printf( "error pointer null\n");



}
void bmp8_printInfo(t_bmp8 *img, bool verbose){

    printf("____________Image info:____________\n\n");
    printf("width = %d\n", img->width);
    printf("height = %d\n", img->height);
    printf("color depth = %d\n", img->colorDepth);
    printf("data size = %d\n", img->dataSize);

    if(verbose){
      int index;
      printf("color table:\n");
      for (int i = 0; i < 256; i++){
        index = i*4;
        printf("rgb(%d, %d, %d) @ %d\n", img->colorTable[index+2], img->colorTable[index+1], img->colorTable[index], index);
      }

        for (int i = 0; i < 54; i++) {
            printf("header[%d] = %02x\n", i, img->header[i]);
        }



    }

}




































//data processing
void bmp8_negative(t_bmp8 * img){
//selon la consigne il faut soustraire a 255 la valeur de chaque pixel, mais on se rappelle qu'on a pas la valeur direct des pixels, on a une lookup table
//La table fait 1024 alors que on est supposé etre sur 8 bits soit 256 couleur nan ? Et ben nan parceque c est 256 pour b, pour g, pour r et pour o, la luminance
//donc il faut adapter la lecture, c est a dire qu'il faut juste pas toucher à o.
    if(!img){
      printf("error pointer null\n");
      return;
    }

    // en fait, il suffit de faire ca
    for(int i = 0; i < img->dataSize; i++) {
        img->data[i] = 255 - img->data[i];
    }



}
void bmp8_brightness(t_bmp8 * img, int value){
//meme idée que pour negatif

    if(!img){
        printf("error pointer null\n");
        return;
    }


    for(int i = 0; i < img->dataSize; i++) {
        img->data[i] = img->data[i] + value > 255 ? 255 : (img->data[i] + value < 0) ? 0 : img->data[i] + value;
    }


}
void bmp8_threshold(t_bmp8 * img, int threshold){
//meme idée que pour negatif

    if(!img){
        printf("error pointer null\n");
        return;
    }


    for(int i = 0; i < img->dataSize; i++) {
        img->data[i] = img->data[i] > threshold ? 255 : 0;
    }

}
void bmp8_applyFilter(t_bmp8 * img, float ** kernel, int kernelSize){

    if(!img){
        printf("error pointer null\n");
        return;
    }
    if(!kernel){
        printf("error kernel null\n");
        return;
    }
    if(kernelSize % 2 == 0){
        printf("kernel size must be odd\n");
        return;
    }

    // Conversion de l'image indexée en image RGB brute
    int* image = (int*)malloc(sizeof(int) * img->dataSize);
    for(int i = 0; i < img->dataSize; i++){
        int index = img->data[i] * 4;
        image[i]= img->colorTable[index];
    }

    // Copie de l'image d'origine
    int* newImage = (int*)malloc(sizeof(int) * img->dataSize);
    for (int i = 0; i < img->dataSize; i++) {
        newImage[i] = image[i];
    }

    int sum;
    int offset = (kernelSize - 1) / 2;

    // Application du filtre
    for(int j = 0; j < img->height; j++){
        for(int i = 0; i < img->width; i++){

            sum=0;

            for(int k = 0; k < kernelSize; k++){
                for(int l = 0; l < kernelSize; l++){
                    int I = i - offset + l;
                    int J = j - offset + k;

                    if(I < 0 || I >= img->width || J < 0 || J >= img->height)
                        continue;

                    int index = J * img->width + I;

                    sum += image[index] * kernel[k][l];

                }
            }


            // on s'assure que sum est bien dans les limites des stocks disponibles
            sum = sum < 0 ? 0 : (sum > 255 ? 255 : sum);

            int index = j * img->width + i;
            newImage[index] = sum;

        }
    }

    // Reprojection des couleurs vers la palette
    for(int i = 0; i < img->dataSize; i++){
        img->data[i] = newImage[i];
    }

    free(image);
    free(newImage);
}



// histogramme stuff
unsigned int* bmp8_computeHistogram(t_bmp8 * img) {

    // on parcourt l'image, et on compte le nombre de pixels pour chaque niveau de gris.

    unsigned int* histogram = (unsigned int*)malloc(sizeof(unsigned int) * 256);
    if (!histogram) {
        printf("error malloc histogram\n");
        return NULL;
    }

    for (int i = 0; i < 256; i++) {
        histogram[i] = 0;
    }

    for(int i = 0; i < img->dataSize; i++) {
        int pixel = img->data[i];
        int index = img->colorTable[pixel * 4]; // *4 car RGBA
        histogram[index]++;
    }

    return histogram;
}
unsigned int* bmp8_computeCDF(unsigned int* hist, unsigned int nb_of_pixels) {

    /**
     * si j'ai bien tt capté, l'histo cumulatif c'est un tableau de même taille que l'histo de base.
     * On remplit la ième case en faisant la somme des i premiers éléments de l'original
     **/

    if (!hist) {
        printf("error pointer null\n");
        return NULL;
    }

    // allocation mémoire
    unsigned int* cdf = (unsigned int*)malloc(sizeof(unsigned int) * 256);
    if (!cdf) {
        printf("error malloc cdf\n");
        return NULL;
    }

    for (int i = 0; i < 256; i++) {
        cdf[i] = 0;
    }

    // calcul du cumulatif
    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i-1] + hist[i];
    }

    // normalisation
    // on commence par trouver le minimum non zéro de cdf
    unsigned int min = 9999999;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] < min && cdf[i] > 0) {
            min = cdf[i];
        }
    }


    for (int i = 0; i < 256; i++) {
        cdf[i] = round(((float)(cdf[i] - min) / (float)(nb_of_pixels - min)) * 255.0f);
    }

    return cdf;
}
void bmp8_equalize(t_bmp8 * img) {

    unsigned int* histo = bmp8_computeHistogram(img);

    unsigned int* cdf = bmp8_computeCDF(histo, img->dataSize);

    for (int i = 0; i < img->dataSize; i++) {
        int original_index = img->data[i];
        int gray = img->colorTable[original_index * 4]; // *4 car RGBA
        int new_index = cdf[gray];
        img->data[i] = new_index;
    }

    /*unsigned int* new_histo = bmp8_computeHistogram(img);
    for (int i = 0; i < 256; i++) {

        if (new_histo[i] != histo[i]) {
            printf("diff histo\n");
        }

    }*/

    free(histo);
    free(cdf);
}
