//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de fun��es n�o seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar mem�ria para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels) {
    IVC *image = (IVC *) malloc(sizeof(IVC));

    if (image == NULL) return NULL;
    if ((levels <= 0) || (levels > 255)) return NULL;

    image->width = width;
    image->height = height;
    image->channels = channels;
    image->levels = levels;
    image->bytesperline = image->width * image->channels;
    image->data = (unsigned char *) malloc(image->width * image->height * image->channels * sizeof(char));

    if (image->data == NULL) {
        return vc_image_free(image);
    }

    return image;
}


// Libertar mem�ria de uma imagem
IVC *vc_image_free(IVC *image) {
    if (image != NULL) {
        if (image->data != NULL) {
            free(image->data);
            image->data = NULL;
        }

        free(image);
        image = NULL;
    }

    return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char *netpbm_get_token(FILE *file, char *tok, int len) {
    char *t;
    int c;

    for (;;) {
        while (isspace(c = getc(file)));
        if (c != '#') break;
        do c = getc(file);
        while ((c != '\n') && (c != EOF));
        if (c == EOF) break;
    }

    t = tok;

    if (c != EOF) {
        do {
            *t++ = c;
            c = getc(file);
        } while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

        if (c == '#') ungetc(c, file);
    }

    *t = 0;

    return tok;
}


long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height) {
    int x, y;
    int countbits;
    long int pos, counttotalbytes;
    unsigned char *p = databit;

    *p = 0;
    countbits = 1;
    counttotalbytes = 0;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos = width * y + x;

            if (countbits <= 8) {
                // Numa imagem PBM:
                // 1 = Preto
                // 0 = Branco
                //*p |= (datauchar[pos] != 0) << (8 - countbits);

                // Na nossa imagem:
                // 1 = Branco
                // 0 = Preto
                *p |= (datauchar[pos] == 0) << (8 - countbits);

                countbits++;
            }
            if ((countbits > 8) || (x == width - 1)) {
                p++;
                *p = 0;
                countbits = 1;
                counttotalbytes++;
            }
        }
    }

    return counttotalbytes;
}


void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height) {
    int x, y;
    int countbits;
    long int pos;
    unsigned char *p = databit;

    countbits = 1;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos = width * y + x;

            if (countbits <= 8) {
                // Numa imagem PBM:
                // 1 = Preto
                // 0 = Branco
                //datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

                // Na nossa imagem:
                // 1 = Branco
                // 0 = Preto
                datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

                countbits++;
            }
            if ((countbits > 8) || (x == width - 1)) {
                p++;
                countbits = 1;
            }
        }
    }
}


IVC *vc_read_image(char *filename) {
    FILE *file = NULL;
    IVC *image = NULL;
    unsigned char *tmp;
    char tok[20];
    long int size, sizeofbinarydata;
    int width, height, channels;
    int levels = 255;
    int v;

    // Abre o ficheiro
    if ((file = fopen(filename, "rb")) != NULL) {
        // Efectua a leitura do header
        netpbm_get_token(file, tok, sizeof(tok));

        if (strcmp(tok, "P4") == 0) {
            channels = 1;
            levels = 1;
        }    // Se PBM (Binary [0,1])
        else if (strcmp(tok, "P5") == 0) channels = 1;                // Se PGM (Gray [0,MAX(level,255)])
        else if (strcmp(tok, "P6") == 0) channels = 3;                // Se PPM (RGB [0,MAX(level,255)])
        else {
#ifdef VC_DEBUG
            printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

            fclose(file);
            return NULL;
        }

        if (levels == 1) // PBM
        {
            if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
                sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1) {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

                fclose(file);
                return NULL;
            }

            // Aloca mem�ria para imagem
            image = vc_image_new(width, height, channels, levels);
            if (image == NULL) return NULL;

            sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
            tmp = (unsigned char *) malloc(sizeofbinarydata);
            if (tmp == NULL) return 0;

#ifdef VC_DEBUG
            printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

            if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata) {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

                vc_image_free(image);
                fclose(file);
                free(tmp);
                return NULL;
            }

            bit_to_unsigned_char(tmp, image->data, image->width, image->height);

            free(tmp);
        } else // PGM ou PPM
        {
            if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
                sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
                sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255) {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

                fclose(file);
                return NULL;
            }

            // Aloca mem�ria para imagem
            image = vc_image_new(width, height, channels, levels);
            if (image == NULL) return NULL;

#ifdef VC_DEBUG
            printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

            size = image->width * image->height * image->channels;

            if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size) {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

                vc_image_free(image);
                fclose(file);
                return NULL;
            }
        }

        fclose(file);
    } else {
#ifdef VC_DEBUG
        printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
    }

    return image;
}


int vc_write_image(char *filename, IVC *image) {
    FILE *file = NULL;
    unsigned char *tmp;
    long int totalbytes, sizeofbinarydata;

    if (image == NULL) return 0;

    if ((file = fopen(filename, "wb")) != NULL) {
        if (image->levels == 1) {
            sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
            tmp = (unsigned char *) malloc(sizeofbinarydata);
            if (tmp == NULL) return 0;

            fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

            totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
            printf("Total = %ld\n", totalbytes);
            if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes) {
#ifdef VC_DEBUG
                fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

                fclose(file);
                free(tmp);
                return 0;
            }

            free(tmp);
        } else {
            fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

            if (fwrite(image->data, image->bytesperline, image->height, file) != image->height) {
#ifdef VC_DEBUG
                fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

                fclose(file);
                return 0;
            }
        }

        fclose(file);

        return 1;
    }

    return 0;
}


// Code start here
int vc_gray_negative(IVC *srcdst) {
    unsigned char *data = (unsigned char *) srcdst->data;
    int width = srcdst->width;
    int height = srcdst->height;
    int bytesperline = srcdst->width * srcdst->channels;
    int channels = srcdst->channels;
    int x, y;
    long int pos;

    // Verificar Erros
    if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
    if (channels != 1) return 0;

    // Inverte a imagem RGB
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos = y * bytesperline + x * channels;
            data[pos] = 255 - data[pos];
        }
    }

    return 1;
}

int vc_rgb_negative(IVC *srcdst) {
    unsigned char *data = (unsigned char *) srcdst->data;
    int width = srcdst->width;
    int height = srcdst->height;
    int bytesperline = srcdst->width * srcdst->channels;
    int channels = srcdst->channels;
    int x, y;
    long int pos;

    // Verificar Erros
    if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
    if (channels != 3) return 0;

    // Inverte a imagem RGB
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos = y * bytesperline + x * channels;
            data[pos] = 255 - data[pos];
            data[pos + 1] = 255 - data[pos + 1];
            data[pos + 2] = 255 - data[pos + 2];
        }
    }

    return 1;
}

int vc_rgb_to_gray(IVC *src, IVC *dst) {
    unsigned char *datasrc = (unsigned char *) src->data;
    int bytesperline = src->width * src->channels;
    int channels = src->channels;
    unsigned char *datadst = (unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y;
    long int pos_src, pos_dst;
    float rf, gf, bf;

    // Verificação de Erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height)) return 0;
    if ((src->channels != 3) || (dst->channels != 1)) return 0;
    if (channels != 3) return 0;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos_src = y * bytesperline + x * channels;
            pos_dst = y * bytesperline_dst + x * channels_dst;

            rf = (float) datasrc[pos_src];
            gf = (float) datasrc[pos_src + 1];
            bf = (float) datasrc[pos_src + 2];

            datadst[pos_dst] = (unsigned char) ((rf * 0.299f) + (gf * 0.587f) + (bf * 0.114f));
        }
    }
    return 1;
}

int vc_rgb_get_red_gray(IVC *srcdst) {
    unsigned char *data = (unsigned char *) srcdst->data;
    int width = srcdst->width;
    int height = srcdst->height;
    int bytesperline = srcdst->width * srcdst->channels;
    int channels = srcdst->channels;
    int x, y;
    long int pos;

    // Verificação de Erros
    if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
    if (channels != 3) return 0;

    // Extrai a componente Red
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos = y * bytesperline + x * channels;

            data[pos + 1] = data[pos]; // Green
            data[pos + 2] = data[pos]; // Blue
        }
    }
    return 1;
}


#define MAX(r, g, b) (r > b? (r > g? r:g): (b>g?b:g))
#define MIN(r, g, b) (r < b? (r < g? r:g): (b<g?b:g))

int vc_rgb_to_hsv(IVC *src, IVC *dst) {
    //Imgem que entra na função
    unsigned char *data_src = (unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    int width = src->width;
    int height = src->height;
    //Imagem que sai da função
    unsigned char *data_dst = (unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;

    int x, y;
    long int pos_src, pos_dst;
    float r, g, b, maximo, saturacao, hue, minimo;

    //verifica se a imagem e valida
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    //verifica tamanho da imagem
    if ((src->width != dst->width) || (src->height != dst->height)) return 0;
    //verifica os canais
    if ((src->channels != 3) || (dst->channels != 3)) return 0;


    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;
            r = (float) data_src[pos_src];
            g = (float) data_src[pos_src + 1];
            b = (float) data_src[pos_src + 2];

            maximo = MAX(r, g, b);
            minimo = MIN(r, g, b);


            if (maximo != 0 && maximo > minimo) {
                saturacao = (float) (maximo - minimo) * 255 / maximo;

                if (maximo == r && g >= b) {
                    hue = 60 * (g - b) / (maximo - minimo);
                }
                if (maximo == r && b > g) {
                    hue = 360 + 60 * (g - b) / (maximo - minimo);
                }
                if (maximo == g) {
                    hue = 120 + 60 * (b - r) / (maximo - minimo);
                }
                if (maximo == b) {
                    hue = 240 + 60 * (r - g) / (maximo - minimo);
                }
            } else {
                saturacao = 0;
                hue = 0;
            }

            data_dst[pos_dst + 2] = (unsigned char) (maximo);
            data_dst[pos_dst + 1] = (unsigned char) (saturacao);
            data_dst[pos_dst] = (unsigned char) (hue);
        }
    }
    return 1;
}

int vc_hsv_segmentation(IVC *src, IVC *dst, float hmin, float hmax, float smin, float smax, float vmin, float vmax) {
    unsigned char *datasrc = (unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    int width = src->width;
    int height = src->height;

    unsigned char *datadst = (unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;

    int x, y;
    long int pos_src, pos_dst;
    float h, s, v;

    //verifica se a imagem e valida
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    //verifica tamanho da imagem
    if ((src->width != dst->width) || (src->height != dst->height)) return 0;
    ////verifica os canais
    //if ((src->channels != 3) || (dst->channels = 1)) return 0;


    hmin = ((float) hmin) / 360 * 255;
    hmax = ((float) hmax) / 360 * 255;

    smin = ((float) smin) / 100 * 255;
    smax = ((float) smax) / 100 * 255;

    vmin = ((float) vmin) / 100 * 255;
    vmax = ((float) vmax) / 100 * 255;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {

            pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

            h = (float) datasrc[pos_src];
            s = (float) datasrc[pos_src + 1];
            v = (float) datasrc[pos_src + 2];


            if (hmin <= h && h <= hmax && smin <= s && s <= smax && vmin <= v && v <= vmax) {
                datadst[pos_dst] = 0;
            } else {
                datadst[pos_dst] = 255;
            }
        }

    }
    return 1;
}

int vc_scale_gray_to_rgb(IVC *src, IVC *dst) {
    //Imgem que entra na função
    unsigned char *data_src = (unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    int width = src->width;
    int height = src->height;

    //Imagem que sai da função
    unsigned char *data_dst = (unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;

    int x, y;
    long int pos_src, pos_dst;

    //verifica se a imagem e valida
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    //verifica tamanho da imagem
    if ((src->width != dst->width) || (src->height != dst->height)) return 0;
    //verifica os canais
    if ((src->channels != 1) || (dst->channels != 3)) return 0;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

            int value = data_src[pos_src];

            if (value >= 0 && value < 64) {
                data_dst[pos_dst] = 0;
                data_dst[pos_dst + 1] = value * 4;
                data_dst[pos_dst + 2] = 255;
            } else {
                if (value >= 64 && value < 128) {
                    data_dst[pos_dst] = 0;
                    data_dst[pos_dst + 1] = 255;
                    data_dst[pos_dst + 2] = 255 - (value - 64) * 4;
                } else {
                    if (value >= 128 && value < 192) {
                        data_dst[pos_dst] = (value - 128) * 4;
                        data_dst[pos_dst + 1] = 255;
                        data_dst[pos_dst + 2] = 0;
                    } else {
                        // if (value >= 192 && value <= 255)
                        data_dst[pos_dst] = 255;
                        data_dst[pos_dst + 1] = 255 - (value - 192) * 4;
                        data_dst[pos_dst + 2] = 0;
                    }
                }
            }
        }
    }
    return 1;
}

int main(void) {
    IVC *image;
    IVC *image2;

    image = vc_read_image("../Images/FLIR/flir-01.pgm");
    image2 = vc_image_new(image->width, image->height, 3, image->levels);

    if (image == NULL) {
        printf("ERROR -> vc_read_image():\n\tFile not Found!\n");
        getchar();
        return 0;
    }


    // Functions Here
    vc_scale_gray_to_rgb(image,image2);

    vc_write_image("vc-0001.pgm", image2);
    vc_image_free(image);
    vc_image_free(image2);

    printf("Press any key to exit... \n");
    getchar();

    return 0;
}
