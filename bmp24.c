#include "bmp24.h"


//debugging
void bmp24_compare_da_images() {

    unsigned char original_buffer[54];
    unsigned char saved_buffer[54];

    FILE* file = fopen("bmp24.bmp", "rb");
    if (file == NULL) {
        printf("Error opening file\n");
        return;
    }
    fread(original_buffer, sizeof(unsigned char), 54, file);
    fclose(file);

    file = fopen("test_save_24.bmp", "rb");
    if (file == NULL) {
        printf("Error opening file\n");
        return;
    }
    fread(saved_buffer, sizeof(unsigned char), 54, file);
    fclose(file);


    for (int i = 0; i < 54; i++) {
        if (original_buffer[i] != saved_buffer[i]) {
            printf("difference spotted @ i:%d", i);
        }
    }



}




//utils
t_pixel ** bmp24_allocateDataPixels (int width, int height) {

    t_pixel ** pixels = (t_pixel**) malloc(height * sizeof(t_pixel*));
    for (int i = 0; i < height; i++) {
        pixels[i] = (t_pixel*) malloc(width * sizeof(t_pixel));
    }

    if (!pixels) {
        printf("Error in memory allocation of the pixel matrix\n");
        return NULL;
    }

    return pixels;
}
void bmp24_freeDataPixels (t_pixel ** pixels, int height) {

    for (int i = 0; i < height; i++) {
        free(pixels[i]);
    }
    free(pixels);

}
t_bmp24 * bmp24_allocate (int width, int height, int colorDepth) {

    t_bmp24* bitmap = (t_bmp24*) malloc(sizeof(t_bmp24));

    if (!bitmap) {
        printf("Error in memory allocation of the bitmap\n");
        return NULL;
    }

    bitmap->width = width;
    bitmap->height = height;
    bitmap->colorDepth = colorDepth;
    bitmap->data = bmp24_allocateDataPixels(width, height);

    return bitmap;

}
void bmp24_free (t_bmp24 * img) {

    bmp24_freeDataPixels(img->data, img->height); // on free image, mais dans image on a un mega tableau de pixel, faut pas oublier de le free aussi
    free(img);

}


void file_rawRead (long position, void * buffer, uint32_t size, size_t n, FILE* file) {
    fseek(file, position, SEEK_SET);
    fread(buffer, size, n, file);
}
void bmp24_readPixelValue (t_bmp24 * image, int x, int y, FILE * file) {
    // je vois pas bien l'interet de la fonction. voyez si vous captez ce qu ils veulent que vous fassiez avec
}
void bmp24_readPixelData (t_bmp24 * bitmap, FILE * file) {
    // c est la que les atheniens s'atteignirent. Il faut lire la matrice pixel et stocker le tt dans bitmap->data. En sachant que data est de type t_pixel
    // un autre soucis: le format bmp 24 bits mets un padding a la fin des lignes pour qu'elles soient multiples de 4. donc faut cheker ca aussi
    // 3eme soucis, la matrice elle est en bottom up. donc faut inverser l'ordre de lecture.
    // dans la matrice on a un pixel représenté de la sorte [b][g][r]. on a donc width * ca sur height ligne

    const int ligne_size = ((bitmap->header_info.width * 3 + 3) / 4) * 4; // calcul de la taille de la ligne pour qu'elle soit modulo 4.
    unsigned char ligne_buffer[ligne_size];


    for (int i = 0; i < bitmap->header_info.height; i++) {
        int ligne = bitmap->header_info.height - 1 - i;

        fseek(file, (int)bitmap->header.offset + ligne * ligne_size, SEEK_SET);
        int read_char = fread(ligne_buffer, 1, ligne_size, file); // on lit toute la ligne d'un coup, valeur + padding rajouté a la fin pour avoir mod 4
        if (read_char != ligne_size) {
            printf("Error reading from file\n");
            return;
        }
        // on parcours la ligne et on extrait les rgb au fur et a mesure.
        for (int j = 0; j < bitmap->header_info.width; j++) {
            int pixelIndex = j * 3;
            bitmap->data[i][j].blue  = ligne_buffer[pixelIndex];
            bitmap->data[i][j].green = ligne_buffer[pixelIndex + 1];
            bitmap->data[i][j].red   = ligne_buffer[pixelIndex + 2];
        }

    }
}
t_bmp24 * bmp24_loadImage (const char * filename) {

    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error in opening file\n");
        return NULL;
    }


    /* Format des infos importante dans un fichier BMP :
    *
    *   Offset (en byte)     |        info
    *   ---------------------|---------------------------------------------
    *          0             | signature du fichier : "BM", permet de cheker qu on a bien une bitmap
    *          2             | taille totale du fichier (en octets)
    *         10             | position de début des données (offset vers les pixels)
    *         18             | largeur de l'image (en pixels)
    *         22             | hauteur de l'image (en pixels)
    *         26             | nombre de plans couleur (doit etre 1)
    *         28             | profondeur de couleur (bits par pixel)
    */




    //on récupère les infos nécessaires a la création de la bitmap.
    uint32_t width, height;
    uint16_t colorDepth;
    file_rawRead(BITMAP_WIDTH, &width, sizeof(unsigned char), 4, file);
    file_rawRead(BITMAP_HEIGHT, &height, sizeof(unsigned char), 4, file);
    file_rawRead(BITMAP_DEPTH, &colorDepth, sizeof(unsigned char), 2, file);


    // on a les infos, on alloue la memoire pour la bitmap.
    t_bmp24* bitmap = bmp24_allocate( (int) width, (int) height, colorDepth);

    // on a la bitmap, on rempli les infos de bitmap->header.
    // les 4 et 2 sortent pas de mon chapeau, c est la taille en bytes des infos, elles sont écrites dans le sujet, le 1 c est la taille d'1 bytes
    file_rawRead(BITMAP_MAGIC, &bitmap->header.type, 1, 2, file);
    file_rawRead(BITMAP_SIZE, &bitmap->header.size, 1, 4, file);
    file_rawRead(BITMAP_RESERVED1, &bitmap->header.reserved1, 1, 2, file);
    file_rawRead(BITMAP_RESERVED2, &bitmap->header.reserved2, 1, 2, file);
    file_rawRead(BITMAP_OFFSET, &bitmap->header.offset, 1, 4, file);

    // on rempli les infos du bitmap->header_info
    bitmap->header_info.size = INFO_SIZE;
    bitmap->header_info.width = (int)width;
    bitmap->header_info.height = (int)height;
    file_rawRead(BITMAP_PLANES, &bitmap->header_info.planes, 1, 2, file);
    file_rawRead(BITMAP_BITS, &bitmap->header_info.bits, 1, 2, file);
    file_rawRead(BITMAP_COMPRESSED, &bitmap->header_info.compression, 1, 4, file);
    file_rawRead(BITMAP_SIZE_RAW, &bitmap->header_info.imagesize, 1, 4, file);
    file_rawRead(BITMAP_XRES, &bitmap->header_info.xresolution, 1, 4, file);
    file_rawRead(BITMAP_YRES, &bitmap->header_info.yresolution, 1, 4, file);
    file_rawRead(BITMAP_N_COLORS, &bitmap->header_info.ncolors, 1, 4, file);
    file_rawRead(BITMAP_IMP_COLORS, &bitmap->header_info.importantcolors, 1, 4, file);



    bmp24_readPixelData(bitmap, file);

    printf("Image data loaded\n");

    fclose(file);

    return bitmap;


}



void bmp24_printInfo(t_bmp24 * bitmap) {
    printf("------Bitmap info:------\n");
    printf("Width: %d\n", bitmap->header_info.width);
    printf("Height: %d\n", bitmap->header_info.height);
    printf("Bits per pixel: %d\n", bitmap->header_info.bits);
    printf("Compression: %d\n", bitmap->header_info.compression);
    printf("Size: %d\n", bitmap->header_info.imagesize);
    /*printf("Planes: %d\n", bitmap->header_info.planes);
    printf("Type: %02x\n", bitmap->header.type);
    printf("reserved 1: %02x\n", bitmap->header.reserved1);
    printf("reserved 2: %02x\n", bitmap->header.reserved2);*/

}



void file_rawWrite (uint32_t position, void * buffer, uint32_t size, size_t n, FILE * file) {
    fseek(file, position, SEEK_SET);
    fwrite(buffer, size, n, file);
}
void bmp24_writePixelValue (t_bmp24 * image, int x, int y, FILE * file) {



    const int ligne_size = ((image->header_info.width * 3 + 3) / 4) * 4; // calcul de la taille de la ligne pour qu'elle soit modulo 4.
    int position = image->header.offset + (image->header_info.height-1 -y) * ligne_size + x*3;

    //printf("offset(%d) + height-1-y(%d) + x(%d)*3(%d)   = position(%d)\n", image->header.offset, image->header_info.height-1-y, x, x*3, position);

    file_rawWrite(position, &image->data[y][x].blue, 1,1, file);
    file_rawWrite(position+1, &image->data[y][x].green, 1, 1, file);
    file_rawWrite(position+2, &image->data[y][x].red, 1, 1, file);
    //printf("writing pixel %d %d value @ %d \n", x, y, position);


}
void bmp24_writePixelData (t_bmp24 * image, FILE * file) {

    for (int i = 0; i < image->header_info.height; i++) {
        for (int j = 0; j < image->header_info.width; j++) {
            int pixelIndex = j * 3;
            bmp24_writePixelValue(image, j, i, file);
        }
    }



}
void bmp24_saveImage (t_bmp24* img, const char * filename) {

    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Error in opening file\n");
        return;
    }
/*
    // je propose de faire comme pour le header des 8bits, on reconstruit le tableau de 54 bytes de long
    // on le rempli proprement, et ensuite on écirt le tt avec un simple fwrite

    unsigned char header[54];

    header[0] = 'B';
    header[1] = 'M';
    header[2] = img->header.size;
    header[3] = img->header.reserved1;

*/



    // on ecrit header dans l'image
    file_rawWrite(BITMAP_MAGIC, &img->header.type, 1, 2, file);
    file_rawWrite(BITMAP_SIZE,  &img->header.size, 1, 4, file);
    file_rawWrite(BITMAP_RESERVED1, &img->header.reserved1, 1, 2, file);
    file_rawWrite(BITMAP_RESERVED2, &img->header.reserved2, 1, 2, file);
    file_rawWrite(BITMAP_OFFSET,    &img->header.offset, 1, 4, file);

    // on refait la meme pour info_header
    file_rawWrite(HEADER_SIZE,   &img->header_info.size, 1, 4, file);
    file_rawWrite(BITMAP_WIDTH, &img->header_info.width, 1, 4, file);
    file_rawWrite(BITMAP_HEIGHT, &img->header_info.height, 1, 4, file);
    file_rawWrite(BITMAP_PLANES, &img->header_info.planes, 1, 2, file);
    file_rawWrite(BITMAP_BITS, &img->header_info.bits, 1, 2, file);
    file_rawWrite(BITMAP_COMPRESSED, &img->header_info.compression, 1, 4, file);
    file_rawWrite(BITMAP_SIZE_RAW, &img->header_info.imagesize, 1, 4, file);
    file_rawWrite(BITMAP_XRES, &img->header_info.xresolution, 1, 4, file);
    file_rawWrite(BITMAP_YRES, &img->header_info.yresolution, 1, 4, file);
    file_rawWrite(BITMAP_N_COLORS, &img->header_info.ncolors, 1, 4, file);
    file_rawWrite(BITMAP_IMP_COLORS, &img->header_info.importantcolors, 1, 4, file);


    bmp24_writePixelData(img, file);

}




//data processing
void bmp24_negative (t_bmp24 * img) {

    //soustratcion a 255 des rgb de l'image

    for (int i = 0; i < img->header_info.height; i++) {
        for (int j = 0; j < img->header_info.width; j++) {
            img->data[i][j].blue =  255 - img->data[i][j].blue;
            img->data[i][j].green = 255 - img->data[i][j].green;
            img->data[i][j].red =   255 - img->data[i][j].red;
        }
    }

}
void bmp24_grayscale (t_bmp24 * img) {

    //on fait une moyenne pondérée de rgb pour chaque pixel
    //on assigne cette moyenne a rgb, comme ca on est en niveau de gris
    //les coefs sortent pas de mon fiac, ca reprend notre sensibilité au différente couleurs
    //0.299 ∙ Red + 0.587 ∙ Green + 0.114 ∙ Blue
    // je multiplie par 1000 les coef puis divise par 1000 le tt pour avoir des opération sur des int, c est BEACOUP plus rapide que des float
    int gray;
    for (int i = 0; i < img->header_info.height; i++) {
        for (int j = 0; j < img->header_info.width; j++) {
            gray = (img->data[i][j].blue * 114 + img->data[i][j].green * 587 + img->data[i][j].red * 299) / 1000;
            img->data[i][j].blue = gray>255 ? 255 : gray;
            img->data[i][j].green = gray>255 ? 255 : gray;
            img->data[i][j].red = gray>255 ? 255 : gray;
        }
    }

}
void bmp24_brightness (t_bmp24 * img, int value) {

    //ajout de value a chaque canal de pixel

    for (int i = 0; i < img->header_info.height; i++) {
        for (int j = 0; j < img->header_info.width; j++) {
            // ce que je fais en dessous c est des operations ternaires nested.
            // on a:  condition ? si oui : si faux=(condition ? si oui : si faux)
            // ce que ca fait c est plafonné a 255 si c est au dessus
            // si c est pas au dessus on plafonne a 0 si c est en dessous,
            // si c est pas en dessous de 0 (ni au dessus de 255) on met la valeur calculée
            img->data[i][j].blue    = (img->data[i][j].blue + value) > 255 ? 255 :((img->data[i][j].blue + value) < 0 ? 0 :  img->data[i][j].blue + value);
            img->data[i][j].green   = (img->data[i][j].green + value) > 255 ? 255 : ((img->data[i][j].green + value) < 0 ? 0 :  img->data[i][j].green + value);
            img->data[i][j].red     = (img->data[i][j].red + value) > 255 ? 255 : ((img->data[i][j].red + value) < 0 ? 0 :  img->data[i][j].red + value);
        }
    }


}

t_pixel bmp24_convolution (t_bmp24 * img, int x, int y, float ** kernel, int kernelSize) {

    t_pixel result;
    result.blue = 0;
    result.green = 0;
    result.red = 0;

    int offset = (kernelSize-1)/2;

    for (int i = 0; i < kernelSize; i++) {
        for (int j = 0; j < kernelSize; j++) {

            int J = x - offset + j;
            int I = y - offset + i;

            if(I < 0 || I >= img->height || J < 0 || J >= img->width)
                continue;


            result.red += img->data[I][J].red * kernel[i][j];
            result.green += img->data[I][J].green * kernel[i][j];
            result.blue += img->data[I][J].blue * kernel[i][j];


        }
    }

    result.red = result.red > 255 ? 255 : (result.red < 0 ? 0 : result.red);
    result.blue = result.blue > 255 ? 255 : (result.blue < 0 ? 0 : result.blue);
    result.green = result.green > 255 ? 255 : (result.green < 0 ? 0 : result.green);


    return result;
}
void bmp24_apply_filter(t_bmp24 * img,float ** kernel, int kernelSize) {

    // comme pour le 8bit, faut créer une copie de l'image pour la prendre en réference
    // la fonction donnée par vos prof attends une bmp24 pas un tableau 2d, donc ca prend plus de mémoire a initialiser mais pas bien grave

    // on alloue la mémoire de la copie
    t_bmp24* copy = (t_bmp24*)malloc(sizeof(t_bmp24));
    copy->data = (t_pixel**) malloc(sizeof(t_pixel*) * img->header_info.height);
    for (int i = 0; i < img->header_info.height; i++) {
        copy->data[i] = (t_pixel*) malloc(sizeof(t_pixel) * img->header_info.width);
    }

    // on fait la copie a proprement parlé
    for (int i = 0; i < img->header_info.height; i++) {
        for (int j = 0; j < img->header_info.width; j++) {
            copy->data[i][j].red = img->data[i][j].red;
            copy->data[i][j].green = img->data[i][j].green;
            copy->data[i][j].blue = img->data[i][j].blue;
        }
    }
    copy->height = img->header_info.height;
    copy->width = img->header_info.width;




    for (int i = 0; i < img->header_info.height; i++) {
        for (int j = 0; j < img->header_info.width; j++) {

            img->data[i][j] = bmp24_convolution(copy, j, i, kernel, kernelSize);

        }
    }


    bmp24_free(copy);

}




//histogramme stuff
unsigned int* bmp24_computeHistogram(t_yuv** img, int height, int width) {

    // vu que on ne fait l'histo que pour y, on peut le faire litéralement comme on a fait pour bmp8

    unsigned int* histo = (unsigned int*) malloc(sizeof(unsigned int) * 256);
    for (int i = 0; i < 256; i++) {
        histo[i] = 0;
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {

            histo[ img[i][j].y ]++;

        }
    }
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += histo[i];
        //printf("histo[%d] = %d\n", i, histo[i]);
    }
    //printf("sum = %d\n", sum);


    return histo;

}
unsigned int* bmp24_computeCDF(unsigned int* histo, int nb_pixels) {

    unsigned int* cdf = (unsigned int*) malloc(sizeof(unsigned int) * 256);
    if (!cdf) {
        printf("error allocating cdf\n");
        return NULL;
    }
    for (int i = 0; i < 256; i++) {
        cdf[i] = 0;
    }

    if (!histo) {
        printf("error: histo is null\n");
        return NULL;
    }

    // on cumule
    cdf[0] = histo[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i-1] + histo[i];
    }


    // on trouve le minimum
    unsigned int min = 99999999;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] < min && cdf[i] > 0) {
            min = cdf[i];
        }
    }

    // on normalise
    for (int i = 0; i < 256; i++) {
        cdf[i] = round(((float)(cdf[i] - min) / (float)(nb_pixels - min)) * 255.0f);
    }


    return cdf;



}


void bmp24_equalize(t_bmp24 * img) {

    // on commence par transformer notre image rgb en image yuv.
    t_yuv** yuv_image = (t_yuv**) malloc(sizeof(t_yuv*) * img->header_info.height);
    if (yuv_image == NULL) {
        printf("malloc failed\n");
        return;
    }
    for (int i = 0; i < img->header_info.height; i++) {
        yuv_image[i] = (t_yuv*) malloc(sizeof(t_yuv) * img->header_info.width);
    }



    // pour les coef et formule entre rgb et yuv et vice versa, ce qui est donné dans le sujet  NE MARCHE PAS
    // après un temps FOU passé a débuggé mes algos je m'en suis rendu compte,
    // donc ce qui est utilisé ici provient du site suivant : https://softpixel.com/~cwright/programming/colorspace/yuv/
    for (int i = 0; i < img->header_info.height; i++) {
        for (int j = 0; j < img->header_info.width; j++) {
            yuv_image[i][j].y = 0.299 * img->data[i][j].red + 0.587 * img->data[i][j].green + 0.114 * img->data[i][j].blue;
            yuv_image[i][j].u = (-0.168736) * img->data[i][j].red - 0.331264 * img->data[i][j].green + 0.5 * img->data[i][j].blue +128;
            yuv_image[i][j].v = 0.5 * img->data[i][j].red - 0.418688 * img->data[i][j].green -0.081312 * img->data[i][j].blue + 128;

            // yuv est aussi limité par 0 255
            yuv_image[i][j].y = yuv_image[i][j].y > 255 ? 255 : (yuv_image[i][j].y < 0 ? 0 : yuv_image[i][j].y);
            yuv_image[i][j].u = yuv_image[i][j].u > 255 ? 255 : (yuv_image[i][j].u < 0 ? 0 : yuv_image[i][j].u);
            yuv_image[i][j].v = yuv_image[i][j].v > 255 ? 255 : (yuv_image[i][j].v < 0 ? 0 : yuv_image[i][j].v);

        }
    }


    // on fait l'histogramme de la composante y de l'image
    unsigned int* histo = bmp24_computeHistogram(yuv_image, img->header_info.height, img->header_info.width);

    // une fois que c est fait on fait le cumulatif normalisé
    unsigned int* cdf = bmp24_computeCDF(histo, img->header_info.height * img->header_info.width);


    // on égalise
    for (int i = 0; i < img->header_info.height; i++) {
        for (int j = 0; j < img->header_info.width; j++) {
            yuv_image[i][j].y = (int)cdf[yuv_image[i][j].y];
        }
    }

    // on repasse en rgb
    for (int i = 0; i < img->header_info.height; i++) {
        for (int j = 0; j < img->header_info.width; j++) {
            img->data[i][j].red =   yuv_image[i][j].y + 1.4075 * (yuv_image[i][j].v - 128);
            img->data[i][j].green = yuv_image[i][j].y  -  0.3455 * (yuv_image[i][j].u - 128) - (0.7169 * (yuv_image[i][j].v - 128));
            img->data[i][j].blue =  yuv_image[i][j].y  + 1.7790 * (yuv_image[i][j].u - 128);



            img->data[i][j].red = img->data[i][j].red > 255 ? 255 : (img->data[i][j].red < 0 ? 0 : img->data[i][j].red);
            img->data[i][j].green = img->data[i][j].green > 255 ? 255 : (img->data[i][j].green < 0 ? 0 : img->data[i][j].green);
            img->data[i][j].blue = img->data[i][j].blue > 255 ? 255 : (img->data[i][j].blue < 0 ? 0 : img->data[i][j].blue);


        }
    }



    for (int i = 0; i < img->header_info.height; i++) {
        free(yuv_image[i]);
    }
    free(yuv_image);
    free(histo);
    free(cdf);
}

