#include <stdio.h>
#include <stdlib.h>

static size_t getfilesize(char *fname)
{
    fpos_t fsize = 0;
    FILE *fp = fopen(fname, "rb"); 
    fseek(fp, 0, SEEK_END); 
    fgetpos(fp, &fsize); 
    fclose(fp);
    return fsize;
}

int main(int argc, const char *argv[])
{
    size_t size = getfilesize("./puts.o");
    FILE *fp  = fopen("./puts.o", "rb");
    FILE *out = fopen("./puts.o.hex", "wb");
    char buf[size];
    if (fread(buf, size, 1, fp) != 1)
        abort();

    int i;
    fputs("static const unsigned char bitcodes[] = {", out);
    for (i = 0; i < size; ++i) {
        unsigned char h = (unsigned char) buf[i];
        if (i % 16 == 0) {
            fputs("\n\t", out);
        }
        fprintf(out, "0x%02x,", h);
    }
    fputs("\n};", out);
    fclose(out);
    fclose(fp);
    return 0;
}
