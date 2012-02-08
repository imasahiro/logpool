#include <stdio.h>
#include <stdlib.h>

static size_t getfilesize(const char *fname)
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
    int i;
    const char *file = argv[1];
    size_t size = getfilesize(file);
    FILE *fp  = fopen(file, "rb");
    FILE *out = fopen("llvm_bc.h", "wb");
    char buf[size];
    if (fread(buf, size, 1, fp) != 1)
        abort();

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
