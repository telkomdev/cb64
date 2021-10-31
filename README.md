## cb64

Base64 Encoding implementation in C Programming Language

Spec: https://datatracker.ietf.org/doc/html/rfc4648#page-5

### Header only
So just copy `cb64.h` to your project, and you ready to go


### Building Example

```shell
$ mkdir build
$ cd build/
$ cmake ..
$ make
$ cd ..
```

Run example binary
```shell
$ ./bin/cb64 ./testdata/octocat.png
```

### Usage

#### Encoding

Encode and Decode Text
```c
#include <stdio.h>
#include <string.h>
#include "cb64.h"

#define EXIT_ERR(msg) { \
        printf("%s\n", msg); \
        exit(-1); \
    }

int get_file_size(size_t* size, FILE* f);

int main(int argc, char** argv) {

    unsigned char* input = "wur0";
    unsigned char* dst = NULL;
    size_t dst_size = 0;
    int encode_ok = encode_b64(input, 0, &dst, &dst_size);
    if (encode_ok != 0)
        EXIT_ERR("error encode text to base64");

    printf("%s\n", dst);
    printf("%lu\n", dst_size);

    unsigned char* dst_decode = NULL;
    size_t dst_size_decode = 0;
    int decode_ok = decode_b64(dst, 0, &dst_decode, &dst_size_decode);
    if (decode_ok != 0)
        EXIT_ERR("error decode text to base64");

    printf("%s\n", dst_decode);
    printf("%lu\n", dst_size_decode);


    free(dst);
    free(dst_decode);
    return 0;
}
```

Encode File
```c
#include <stdio.h>
#include <string.h>
#include "cb64.h"

#define EXIT_ERR(msg) { \
        printf("%s\n", msg); \
        exit(-1); \
    }

int get_file_size(size_t* size, FILE* f);

int main(int argc, char** argv) 
{

    // encode file to base64

    if (argc < 2)
    {
        EXIT_ERR("required filename");
    }

    char* filename = argv[1];
    char* filename_out = "../testdata/out.txt";

    printf("filename : %s\n", filename);

    FILE* file;
    FILE* file_out;

    file = fopen(filename, "rb");
    if (file == NULL)
    {
        EXIT_ERR("file not found");
    }

    file_out = fopen(filename_out, "wb");

    size_t file_size;
    if (get_file_size(&file_size, file) != 0)
        EXIT_ERR("error get file size");

    printf("%lu\n", file_size);

    unsigned char input[file_size+1];
    unsigned char* dst = NULL;
    size_t dst_size = 0;

    fread(input, sizeof(char), file_size, file);
    input[file_size] = '\0';

    int encode_ok = encode_b64(input, file_size, &dst, &dst_size);
    if (encode_ok != 0)
        EXIT_ERR("error encode text to base64");

    // printf("%s\n", dst);
    fwrite(dst, 1, dst_size, file_out);

    fclose(file);
    fclose(file_out);
    free(dst);

    return 0;
}

int get_file_size(size_t* size, FILE* f)
{
    // goto the end of the file
    if (fseek(f, 0L, SEEK_END) != 0)
        return -1;

    // get the current file position of the given stream.
    *size = ftell(f);

    // go back the start of the file
    if (fseek(f, 0L, SEEK_SET) != 0)
        return -1;
    
    return 0;
}
```

Decode File from Base64 Text
```c
#include <stdio.h>
#include <string.h>
#include "cb64.h"

#define EXIT_ERR(msg) { \
        printf("%s\n", msg); \
        exit(-1); \
    }

int get_file_size(size_t* size, FILE* f);

int main(int argc, char** argv) 
{
    char* filename = "../testdata/haha.txt";
    char* filename_out = "../testdata/out.png";
    FILE* file;
    FILE* file_out;

    file = fopen(filename, "r");
    if (file == NULL)
    {
        EXIT_ERR("file not found");
    }

    file_out = fopen(filename_out, "wb");

    size_t file_size;
    if (get_file_size(&file_size, file) != 0)
        EXIT_ERR("error get file size");

    printf("%lu\n", file_size);

    unsigned char input[file_size+1];

    unsigned char* dst_decode = NULL;
    size_t dst_size_decode = 0;

    fread(input, sizeof(char), file_size, file);
    input[file_size] = '\0';

    int decode_ok = decode_b64(input, 0, &dst_decode, &dst_size_decode);
    if (decode_ok != 0)
        EXIT_ERR("error decode text to base64");

    printf("%s\n", dst_decode);

    fwrite(dst_decode, 1, dst_size_decode, file_out);

    free(dst_decode);
    fclose(file);
    fclose(file_out);
    return 0;
}

int get_file_size(size_t* size, FILE* f)
{
    // goto the end of the file
    if (fseek(f, 0L, SEEK_END) != 0)
        return -1;

    // get the current file position of the given stream.
    *size = ftell(f);

    // go back the start of the file
    if (fseek(f, 0L, SEEK_SET) != 0)
        return -1;
    
    return 0;
}
```