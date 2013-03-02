#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FBE_SIGN_SIZE 4
#define FBE_SIZEOFFILE_SIZE 4
#define FBE_UNKW_SIZE 4
#define FBE_FILENUMBER_SIZE 4


typedef struct FBE_Header
{
    char sign[4];
    int sizeoffile;
    int unkw;
    int filenumber;
    int* pointertable;
    int* sizefile;
}FBE_Header;

int file_number(char* name);
void FBE_build_body(FBE_Header* head, char* name);
void FBE_build_file(FBE_Header* head, FILE* fbe);

int main(int argc, char* argv[])
{
    FBE_Header head;
    FILE* fbe = NULL;
    
    if(argc != 3)
    {
        printf("Usage: fbe-rebuilder fileout.fbe filelist.txt\n");
        return 0;
    }
    
    fbe = fopen(argv[1], "wb+");
    
    if(!fbe)
    {
        printf("Unable to open %s\n", argv[1]);
        exit(-1);
    }
    
    head.filenumber = file_number(argv[2]);
    head.unkw = 0x00000004;
    head.pointertable = calloc(head.filenumber+1, sizeof(int));
    if(!head.pointertable)
    {
        printf("Bad alloc\n");
        exit(-1);
    }
    head.sizefile = calloc(head.filenumber+1, sizeof(int));
    if(!head.sizefile)
    {
        free(head.pointertable);
        printf("Bad alloc\n");
        exit(-1);
    }
    
    FBE_build_body(&head, argv[2]);
    FBE_build_file(&head, fbe);
    
    fclose(fbe);
    free(head.sizefile);
    free(head.pointertable);
    
    return 0;
}

void FBE_build_file(FBE_Header* head, FILE* fbe)
{
    FILE* body = fopen("tmp.tmp", "rb");
    int i = 0;
    char sign[] = {0x46, 0x42, 0x45, 0x10};
    
    fwrite(&sign[0], sizeof(char), 1, fbe);
    fwrite(&sign[1], sizeof(char), 1, fbe);
    fwrite(&sign[2], sizeof(char), 1, fbe);
    fwrite(&sign[3], sizeof(char), 1, fbe);
    fwrite(&head->sizeoffile, sizeof(char), 4, fbe);
    fwrite(&head->unkw, sizeof(char), 4, fbe);
    fwrite(&head->filenumber, sizeof(char), 4, fbe);
    
    for(int a = 0; a < head->filenumber; a++)
    {
        fwrite(&head->pointertable[a], sizeof(char), 4, fbe);
        fwrite(&head->sizefile[a], sizeof(char), 4, fbe);
    }
    
    while(!feof(body))
    {
        fread(&i, sizeof(char), 1, body);
        fwrite(&i, sizeof(char), 1, fbe);
    }
    
    fclose(body);
    
    remove("tmp.tmp");
}        

void FBE_build_body(FBE_Header* head, char* name)
{
    int i = 0;
    int j = 0;
    FILE* body = fopen("tmp.tmp", "wb+");
    FILE* fn = fopen(name, "r");
    FILE* fp = NULL;
    char string[200] = {0};
    int size = 0x10+8*head->filenumber;
    
    while(fgets(string, 200, fn) != NULL)
    {
        string[strlen(string)-1] = '\0';
        fp = fopen(string, "rb");
        
        if(!fp)
        {
            printf("Unable to open %s\n", string);
            free(head->pointertable);
            free(head->sizefile);
            remove("tmp.tmp");
            exit(-1);
        }
        head->pointertable[j] = size;
        while(!feof(fp))
        {
            fread(&i, sizeof(char), 1, fp);
            fwrite(&i, sizeof(char), 1, body);
            size++;
        }
        head->sizefile[j] = size - head->pointertable[j];
        fclose(fp);
        j++;
    }
    
    head->sizeoffile = size;
    fclose(fn);
    fclose(body);
}

int file_number(char* name)
{
    FILE* in = fopen(name, "r");
    int i = 0;
    
    if(!in)
    {
        printf("Unable to open %s \n", name);
        exit(-1);
    }
    
    char string[200] = {0};
    
    while(fgets(string, 200, in) != NULL)
    {
        i++;
    }
    
    return i;
}
