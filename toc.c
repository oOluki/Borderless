#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int find_index(uint32_t color, stbi_uc* pixels, int w, int h, int comp){

    if(pixels == NULL) return -1;

    for(int i = 0; i < h; i+=1){
        for(int j = 0; j < w; j+=1){
            switch (comp)
            {
            case 1:
                if((uint32_t) ((uint8_t*)(pixels))[i * w + j] == color)
                    return i * w + j;
                break;
            case 2:
                if((uint32_t) ((uint16_t*)(pixels))[i * w + j] == color)
                    return i * w + j;
                break;
            case 4:
                if((uint32_t) ((uint32_t*)(pixels))[i * w + j] == color)
                    return i * w + j;
                break;
            
            default:
                return -1;
            }
        }
    }
    return -1;
}

int get(stbi_uc** src, void* dest, int comp){
    switch (comp)
    {
    case 1:
        *((uint8_t*)(dest)) = *((uint8_t*)(*src));
        *(uint8_t**)(src)  += 1;
        return 0;
    case 2:
        *((uint16_t*)(dest)) = *((uint16_t*)(*src));
        *(uint16_t**)(src)  += 1;
        return 0;
    case 4:
        *((uint32_t*)(dest)) = *((uint32_t*)(*src));
        *(uint32_t**)(src)  += 1;
        return 0;
    
    default:
        return 1;
    }
    return 0;
}

int main(int argc, char** argv){

    if(argc < 2){
        fprintf(stderr, "[ERROR] Expected Input Png\n");
        return 1;
    }

    int w;
    int h;
    int comp;
    stbi_uc* pixels = stbi_load(argv[1], &w, &h, &comp, 0);
    int n = 0;
    

    if(!pixels){
        fprintf(stderr, "[ERROR] Unable To Load '%s': %s\n", argv[1], stbi_failure_reason());
        return 1;
    }
    if(comp != 1 && comp != 2 && comp != 4){
        stbi_image_free(pixels);
        fprintf(stderr, "[ERROR] Invalid Compression %i For '%s'\n", comp, argv[1]);
        return 1;
    }

    int pw;
    int ph;
    int pcomp;
    stbi_uc* ppixels = NULL;
    int pn = 0;

    if(argc > 2){
        ppixels = stbi_load(argv[2], &pw, &ph, &pcomp, 0);
        if(!ppixels){
            stbi_image_free(pixels);
            fprintf(stderr, "[ERROR] Unable To Load '%s': %s\n", argv[2], stbi_failure_reason());
            return 1;
        }
        if(pcomp != 1 && pcomp != 2 && pcomp != 4){
            stbi_image_free(pixels);
            stbi_image_free(ppixels);
            fprintf(stderr, "[ERROR] Invalid Compression %i For '%s'\n", comp, argv[2]);
            return 1;
        }
        int dummy = pw * ph;
        while (dummy)
        {
            dummy /= 10;
            pn+=1;
        }        
    }


    printf("static const int w = %i;\n", w);
    printf("static const int h = %i;\n", h);

    if(!ppixels){
        switch (comp)
        {
        case 1:
            printf("static const uint8_t array[] = {\n");
            break;
        case 2:
            printf("static const uint16_t array[] = {\n");
            break;
        case 4:
            printf("static const uint32_t array[] = {\n");
            break;
        default:
            break;
        }
        ppixels = pixels;
        uint32_t o = 0;
        for(int i = 0; i < h; i+=1){
            printf("\t");
            for(int j = 0; j < w - 1; j+=1){
                get(&ppixels, &o, comp);
                printf("0x%.*x, ", comp * 2, o);
            }
            get(&ppixels, &o, comp);
            printf("0x%.*x,\n", comp * 2, o);
        }
    }
    else {
        printf("static const int array[] = {\n");
        stbi_uc* p = pixels;
        uint32_t color = 0;
        for(int i = 0; i < h; i+=1){
            printf("\t");
            for(int j = 0; j < w - 1; j+=1){
                get(&p, &color, comp);
                const int o = find_index(color, ppixels, pw, ph, pcomp);
                if(o < 0){
                    stbi_image_free(pixels);
                    stbi_image_free(ppixels);
                    fprintf(stderr, "[ERROR] Could Not Find %" PRIu32 " From '%s' In '%s'\n", color, argv[1], argv[2]);
                    return 1;
                }
                printf("%.*u, ", pn, o);
            }
            get(&p, &color, comp);
            const int o = find_index(color, ppixels, pw, ph, pcomp);
            if(o < 0){
                stbi_image_free(pixels);
                stbi_image_free(ppixels);
                fprintf(stderr, "[ERROR] Could Not Find %" PRIu32 " From '%s' In '%s'\n", color, argv[1], argv[2]);
                return 1;
            }
            printf("%.*u,\n", pn, o);
        }
    }

    printf("};\n");

    stbi_image_free(pixels);

    return 0;
}
