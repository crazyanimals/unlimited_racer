#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif


#include <stdio.h>
#include <string.h>
#include <malloc.h>

FILE *fw;

typedef unsigned long DWORD;

char * pMemoryBuffer = NULL;
DWORD dwMemoryBufferSize = 0;


int PrintHelp() { 
    printf("Syntax :\nModelMaker.exe output_filename XFile_1 XFile_2 XFile_3 ... XFile_N texture_1 texture_2 ... texture_n\n\n");
    return 0;
}

int PrintHeader(int argc, char * argv[]) {
    int i;
    DWORD dwFileCount = 0, dwFileSize;
    void * lpTmp;
    FILE *fr;
    char name[256];
    printf("Checking files ...\n");
    for (i = 2; i < argc; i++) {
        printf("Opening file '%s' - ", argv[i]);
        fr = fopen(argv[i], "rb");
        if (!fr) {
            printf("FAILED!\n");
            continue;
        }
        printf("OK.\n");
        dwFileCount++;
        fclose(fr);
    }
    printf("\nAdding header information...\n");
    fwrite(&dwFileCount, sizeof(DWORD), 1, fw);
    for (i = 2; i < argc; i++) {
        printf("File '%s' - ", argv[i]);
        fr = fopen(argv[i], "rb");
        if (!fr) {
            printf("SKIPPED!\n");
            continue;
        }
        strncpy(name, argv[i], 256);
        name[255] = 0;
        fwrite(name, sizeof(char), 256, fw);
        fseek(fr, 0, SEEK_END);
        dwFileSize = ftell(fr);
        if (dwFileSize > dwMemoryBufferSize) {
            lpTmp = realloc(pMemoryBuffer, dwFileSize);
            if (!lpTmp) {
                free(pMemoryBuffer);
                printf("\n\nNot enough memory. Buffer allocation error. Application terminated.\n");
                return 0;
            }
            pMemoryBuffer = (char *) lpTmp;
            dwMemoryBufferSize = dwFileSize;
        }
        fwrite(&dwFileSize, sizeof(DWORD), 1, fw);
        fclose(fr);
        printf("HEADER ADDED.\n");
    }
    return 1;
}

void PrintFiles(int argc, char * argv[]) {
    int i;
    DWORD dwFileSize;
    FILE *fr;
    printf("\nAdding files ...\n");
    for (i = 2; i < argc; i++) {
        printf("File '%s' - ", argv[i]);
        fr = fopen(argv[i], "rb");
        if (!fr) {
            printf("SKIPPED!\n");
            continue;
        }
        fseek(fr, 0, SEEK_END);
        dwFileSize = ftell(fr);
        fseek(fr, 0, SEEK_SET);
        fread(pMemoryBuffer, 1, dwFileSize, fr);
        fwrite(pMemoryBuffer, 1, dwFileSize, fw);
        fclose(fr);
        printf("FILE APPENDED! (%li Bytes)\n", dwFileSize);
    }
    printf("\n\nDone!\n");
}

int main(int argc, char * argv[]) {
    int p = 0, len;
    char * sOutputFile;

    if (argc < 2) return PrintHelp();

    len = (int) strlen(argv[1]);
    if (len < 2) return PrintHelp();

    sOutputFile = argv[1];

    fw = fopen(sOutputFile, "wb");
    if (!fw) {
        printf("Could not open '%s' for writing!", sOutputFile);
        return 1;
    }
        
    if (!PrintHeader(argc, argv)) return 1;
    PrintFiles(argc, argv);

    fclose(fw);
    free(pMemoryBuffer);

    return 0;
    
}