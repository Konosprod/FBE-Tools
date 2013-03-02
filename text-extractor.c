#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Header
{
    int nbstring;
    int sizetable;
    int beg_table;
    int* table;
}Header;

void get_head(Header* head, FILE* in);
void dump_file(Header* head, FILE* in, char* name);

int main(int argc, char* argv[])
{
    FILE* in = NULL;
    Header head;
    head.nbstring = 0;
    head.sizetable = 0;
    head.beg_table = 0;
    
    if(argc != 2)
    {
        printf("./extract-text [FILE]\n");
        exit(EXIT_FAILURE);
    }
    
    in = fopen(argv[1], "rb+");
    
    if(!in)
    {
        printf("Unable to open %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    
    get_head(&head, in);
    dump_file(&head, in, argv[1]);
    
    free(head.table);
    
    return EXIT_SUCCESS;
}

void dump_file(Header* head, FILE* in, char* name)
{
    FILE* out = NULL;
    char filename[50] = {0};
    char rc[] = "</para>\n";
    int sz = strlen(rc);
    char a = 0;
    int size = 0;
    
    
    
    sprintf(filename, "%s.ext", name);
    
    out = fopen(filename, "wb+");
    
    if(!out)
    {
        printf("Unable to open %s\n", filename);
        return;
    }
    
    for(int i = 0; i < head->nbstring; i++)
    {
        size = head->table[i+1] - head->table[i];
        fseek(in, head->table[i], SEEK_SET);
        for(int j = 0; j < size-1; j++)
        {
            fread(&a, sizeof(char), 1, in);
            fwrite(&a, sizeof(char), 1, out);
        }
        fwrite(&rc, sizeof(char), sz, out);
    }
}

void get_head(Header* head, FILE* in)
{
    fread(&head->nbstring, sizeof(char), 2, in);
    printf("Strings: %d\n", head->nbstring);
    
    head->beg_table = head->nbstring*0x02 + 0x06;
    printf("Offset table: 0x%.8X\n", head->beg_table);
    
    fread(&head->sizetable, sizeof(char), 4, in);
    printf("Bloc size: 0x%.8X\n", head->sizetable);
    
    head->table = calloc(head->nbstring+1, sizeof(int));
    
    if(!head->table)
    {
        printf("Bad alloc\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < head->nbstring; i++)
    {
        fread(&head->table[i], sizeof(char), 2, in);
        head->table[i]+=head->beg_table;
        printf("Table[%d]:\t0x%.8X\n", i, head->table[i]);
    }
    
    head->table[head->nbstring] = head->sizetable + head->beg_table;
    printf("Table[%d]:\t0x%.8X\n", head->nbstring, head->table[head->nbstring]);
}
    
    
