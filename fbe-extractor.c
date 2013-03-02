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

void FBE_get_header(FILE* fbe, FBE_Header* head);
void FBE_dump_file(FILE* fbe, FBE_Header* head);
int test_sign_spt(char* sign);
int test_sign_fbe(char* sign);

int main(int argc, char* argv[])
{
    FBE_Header header;
    FILE* in = NULL;
    
    if(argc != 2)
    {
        printf("Usage: ./fbe-extractor file.fbe\n");
        return 0;
    }
    else
    {
        in = fopen(argv[1], "rb");
        FBE_get_header(in, &header);
        if(test_sign_fbe(header.sign) != 0)
        {
            printf("Bad FBE file, signature should be: 0x4642 4510\n");
            exit(-1);
        }
        FBE_dump_file(in, &header);
        fclose(in);
    }
    free(header.pointertable);
    free(header.sizefile);
    return 0;
}

void FBE_get_header(FILE* fbe, FBE_Header* head)
{
    int i = 0;
    
    fread(&head->sign, sizeof(char), FBE_SIGN_SIZE, fbe);
    fread(&head->sizeoffile, sizeof(char), FBE_SIZEOFFILE_SIZE, fbe);
    fread(&head->unkw, sizeof(char), FBE_UNKW_SIZE, fbe);
    fread(&head->filenumber, sizeof(char), FBE_FILENUMBER_SIZE, fbe);
    
    printf("SIZE: 0x%.8X\n", head->sizeoffile);
    printf("UNKW: 0x%.8X\n", head->unkw);
    printf("FILE: 0x%.8X\n", head->filenumber);
    
    head->pointertable = calloc(head->filenumber+1, sizeof(int));
    head->sizefile = calloc(head->filenumber+1, sizeof(int));
    
    if(!head->pointertable)
    {
        printf("Bad alloc\n");
        exit(-1);
    }
    if(!head->sizefile)
    {
        printf("Bad alloc\n");
        free(head->pointertable);
        exit(-1);
    }
    
    for(i = 0; i < head->filenumber; i++)
    {
        fread(&head->pointertable[i], sizeof(char), 4, fbe);
        fread(&head->sizefile[i], sizeof(char), 4, fbe);
        printf("File[%d]: offset: 0x%.8X\tsize: 0x%.8X\n", i, 
        head->pointertable[i], head->sizefile[i]);
    }
}

void FBE_dump_file(FILE* fbe, FBE_Header* head)
{
    int i = 0;
    int j = 0;
    int a = 0;
    FILE* out = NULL;
    char name[20] = {0};
    
    for(i = 0; i < head->filenumber; i++)
    {
        
        fseek(fbe, head->pointertable[i], SEEK_SET);
        fread(&j, sizeof(char), 4, fbe);
        
        if(j == 0x46424510 || j == 0x10454246)
        {
            sprintf(name, "%d.fbe", i);
            
        }
        else if(j == 0x2E535054 || j == 0x5450532E)
        {
            sprintf(name, "%d.spt", i);
        }
        else
        {
            sprintf(name, "%d.bin", i);
        } 
        
        //printf("File[%d]: %s....", i, name);
        
        out = fopen(name, "wb+");
        
        fwrite(&j, sizeof(char), 4, out);
        
        while(a < head->sizefile[i]-4)
        {
            fread(&j, sizeof(char), 1, fbe);
            fwrite(&j, sizeof(char), 1, out);
            a++;
        }
        a = 0;
        //printf("...dumped !\n");
        fclose(out);
    }
}
        
        
int test_sign_fbe(char* sign)
{
    char signatures[4] = {0x46, 0x42, 0x45, 0x10};
    int i = 0;
    
    for(i =  0; i < 4; i++)
    {
        if(sign[i] != signatures[i])
        {
            return -1;
        }
    }
    return 0;
}

int test_sign_spt(char* sign)
{
    char signatures[4] = {0x2E, 0x53, 0x50, 0x54};
    int i = 0;
    
    for(i =  0; i < 4; i++)
    {
        if(sign[i] != signatures[i])
        {
            return -1;
        }
    }
    return 0;
}
