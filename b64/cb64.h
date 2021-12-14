#ifndef HEADER_CB64_H
#define HEADER_CB64_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#define TRUE 1
#define FALSE 0

#define B64_TABLE_SIZE 64

#define B64_ENCODE_OK 0
#define B64_ENCODE_FAIL -1
#define B64_DECODE_OK 0
#define B64_DECODE_FAIL -1

struct base64_table_dict
{
    char key;
    uint8_t val;
} b64_dict_t;

static struct base64_table_dict dict[B64_TABLE_SIZE];

static const uint8_t BASE_64_TABLE[B64_TABLE_SIZE] = {
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 
    0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 
    0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 
    0x59, 0x5A, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 
    0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 
    0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 
    0x77, 0x78, 0x79, 0x7A, 0x30, 0x31, 0x32, 0x33, 
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2B, 0x2F
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

static struct base64_table_dict base64_table_dict_find(char key)
{
    for (int i = 0; i < B64_TABLE_SIZE; i++)
    {
        if (dict[i].key == key)
            return dict[i];
    }
    return b64_dict_t;
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

int encode_b64(const unsigned char* src, size_t src_size, 
    unsigned char** dst, size_t* dst_size)
{
    if (src_size == 0)
        src_size = size_char_ptr(src);
    
    uint32_t _dst_size = 
        (uint32_t)(ceil((double) src_size/(double) 3) + (double) src_size);

    // caller should call free()
    unsigned char* _dst = (unsigned char*) malloc(sizeof(*_dst) * _dst_size + 2);
    if (_dst == NULL)
        return B64_ENCODE_FAIL;

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

            uint8_t c_b64 = BASE_64_TABLE[idx_b];

            fputc(c_b64, base64_res_f);
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

    return B64_ENCODE_OK;
}

int decode_b64(const unsigned char* src, size_t src_size, 
    unsigned char** dst, size_t* dst_size)
{
    init_b64_table_dict();
    if (src_size == 0)
        src_size = size_char_ptr(src);
    
    uint32_t _dst_size = 
        (uint32_t)((ceil((double) src_size/(double) 4)) * (double) 3);

    // caller should call free()
    unsigned char* _dst = (unsigned char*) malloc(sizeof(*_dst) * _dst_size);
    if (_dst == NULL)
        return B64_DECODE_FAIL;

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

            struct base64_table_dict b64_idx_dict = base64_table_dict_find(buffer[i]);
            uint32_t l_shift = 18 - segment_count * 6;
            uint32_t b64_idx = (uint32_t) b64_idx_dict.val;
            dec |= b64_idx << l_shift;
            segment_count = segment_count + 1;
        }

        for (int i = 0; i < segment_count-1; i++)
        {
            uint32_t r_shift = 16 - i * 8;
            uint8_t c_b255 = (uint8_t) (dec >> r_shift) & EIGHT_BIT_MASK;

            fputc(c_b255, text_f);
            size_out = size_out + 1;
        }

    }

    // terminate the buffer with '\0'
    _dst[size_out] = '\0';
    *dst = _dst;
    *dst_size = size_out;

    fclose(base64_in_f);
    fclose(text_f);
    
    return B64_DECODE_OK;
}

#endif