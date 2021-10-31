#ifndef HEADER_CB64_H
#define HEADER_CB64_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define TRUE 1
#define FALSE 0

#define B64_TABLE_SIZE 64

struct base64_table_dict
{
    char key;
    uint8_t val;
} b64_dict_t;

static struct base64_table_dict dict[B64_TABLE_SIZE];

static const uint8_t BASE_64_TABLE[B64_TABLE_SIZE] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 
    'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 
    'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', 
    '4', '5', '6', '7', '8', '9', '+', '/'
};

static const uint8_t PADDING = 0x3D;
static const uint8_t SIX_BIT_MASK = 0x3F; // 63 // 111111
static const uint8_t EIGHT_BIT_MASK = 0xFF; // 255 // 11111111

static void init_b64_table_dict()
{
    for (int i = 0; i < B64_TABLE_SIZE; i++)
    {
        b64_dict_t.key = BASE_64_TABLE[i];
        b64_dict_t.val = i;
        dict[i] = b64_dict_t;
    }
}

static uint8_t base64_table_dict_find(char key)
{
    for (int i = 0; i < B64_TABLE_SIZE; i++)
	{
        if (dict[i].key == key)
            return i;
    }
    return 0;
}

static uint64_t size_char_ptr(const unsigned char* arr)
{
    uint64_t size = 0;

    while(*arr)
    {
        size++;
        arr++;
    }

    return size;
}

int encode_b64(const unsigned char* src, size_t src_size, unsigned char** dst, size_t* dst_size)
{
    if (src_size == 0)
        src_size = size_char_ptr(src);
    
    uint32_t _dst_size = (uint32_t)(ceil((double) src_size/(double) 3) + (double) src_size);

    // caller should call free()
    unsigned char* _dst = (unsigned char*) malloc(sizeof(*_dst) * _dst_size + 2);
    if (_dst == NULL)
        return -1;

    FILE* f = fmemopen((void*)src, src_size, "r");
    FILE* base64_res_f = fmemopen((void*)_dst, sizeof(*_dst) * _dst_size + 2, "w");

    unsigned char buffer[4];
    uint32_t size_out = 0;
    uint32_t total_read = 0;

    while(TRUE)
    {
        // read data per 3 bytes to buffer
        size_t r = fread(buffer, 1, 3, f);

        // check if the entire buffer has been read
        if (r <= 0 || total_read == src_size)
            break;
        
        total_read += r;
        
        // terminate the buffer with '\0'
        buffer[r] = '\0';

        uint32_t segment_count = 0;
        uint32_t dec = 0;
        for (int i = 0; i < r; i++)
        {
            // if (buffer[i] == '\0') 
            //     continue;
            
            uint32_t l_shift = 16 - segment_count * 8;
            uint32_t s = (uint32_t) buffer[i];
            dec |= s << l_shift;
            segment_count = segment_count + 1;
        }

        for (int i = 0; i < segment_count+1; i++)
        {
            uint32_t r_shift = 18 - i * 6;
            uint8_t idx_b = (uint8_t) (dec >> r_shift) & SIX_BIT_MASK;

            uint8_t c = BASE_64_TABLE[idx_b];

            fputc(c, base64_res_f);
            size_out = size_out + 1;
        }
        if (segment_count == 2)
        {
            fputc(PADDING, base64_res_f);
            size_out = size_out + 1;
        }
        if (segment_count == 1)
        {
            fputc(PADDING, base64_res_f);
            fputc(PADDING, base64_res_f);
            size_out = size_out + 2;
        }

    }
    // terminate the buffer with '\0'
    _dst[size_out] = '\0';
    *dst = _dst;
    *dst_size = size_out;
    
    fclose(f);
    fclose(base64_res_f);
    return 0;
}

int decode_b64(const unsigned char* src, size_t src_size, unsigned char** dst, size_t* dst_size)
{
    init_b64_table_dict();
    if (src_size == 0)
        src_size = size_char_ptr(src);
    
    uint32_t _dst_size = (uint32_t)((ceil((double) src_size/(double) 4)) * (double) 3);

    // caller should call free()
    unsigned char* _dst = (unsigned char*) malloc(sizeof(*_dst) * _dst_size);
    if (_dst == NULL)
        return -1;

    FILE* base64_in_f = fmemopen((void*)src, src_size, "r");
    FILE* text_f = fmemopen((void*)_dst, sizeof(*_dst) * _dst_size, "w");

    unsigned char buffer[5];
    uint32_t size_out = 0;
    uint32_t total_read = 0;

    while (TRUE)
    {
        // read data per 4 bytes to buffer
        size_t r = fread(buffer, 1, 4, base64_in_f);

        // check if the entire buffer has been read
        if (r <= 0 || total_read == src_size)
            break;
        
        total_read += r;
        
        // terminate the buffer with '\0'
        buffer[r] = '\0';

        uint32_t segment_count = 0;
        uint32_t dec = 0;
        for (int i = 0; i < 4; i++)
        {
            if (buffer[i] == '\0' || buffer[i] == PADDING) 
                continue;

            uint64_t b64_idx = (uint64_t) base64_table_dict_find(buffer[i]);
            uint32_t l_shift = 18 - segment_count * 6;
            dec |= b64_idx << l_shift;
            segment_count = segment_count + 1;
        }

        for (int i = 0; i < segment_count-1; i++)
        {
            uint32_t r_shift = 16 - i * 8;
            uint8_t code_point = (uint8_t) (dec >> r_shift) & EIGHT_BIT_MASK;

            fputc(code_point, text_f);
            size_out = size_out + 1;
        }

    }

    // terminate the buffer with '\0'
    _dst[size_out] = '\0';
    *dst = _dst;
    *dst_size = size_out;

    fclose(base64_in_f);
    fclose(text_f);
    
    return 0;
}

#endif