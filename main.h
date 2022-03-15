//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define VC_DEBUG


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
    unsigned char *data;
    int width, height;
    int channels;			// Bin�rio/Cinzentos=1; RGB=3
    int levels;				// Bin�rio=1; Cinzentos [1,255]; RGB [1,255]
    int bytesperline;		// width * channels
} IVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROT�TIPOS DE FUN��ES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

// FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);

// Code start here
int vc_gray_negative(IVC *srcdst);
int vc_rgb_negative(IVC *srcdst);

int vc_rgb_get_red_gray(IVC *srcdst);
int vc_rgb_to_hsv(IVC *src, IVC *dst);
int vc_hsv_segmentation(IVC* src, IVC* dst, float hmin, float hmax, float smin, float smax, float vmin, float vmax);
int vc_scale_gray_to_rgb(IVC *src, IVC *dst);