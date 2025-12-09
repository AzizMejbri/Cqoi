#include "decode.h"

#include <stdlib.h>
#include <assert.h>

#include <pretty.h>


#define hash(p) ( (p.r * 3 + p.g * 5 + p.b * 7 + 255 * 11) & 63 )

#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x)   __builtin_expect(!!(x), 1)

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define IS_LITTLE_ENDIAN 1
        #define IS_BIG_ENDIAN 0
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define IS_LITTLE_ENDIAN 0
        #define IS_BIG_ENDIAN 1
    #endif
#endif

#if IS_LITTLE_ENDIAN == 1 
  #define bytes_to_u32(buffer) (*(buffer) | *(buffer + 1) << 8 | *(buffer + 2) << 16 | *(buffer + 3) << 24)
#else 
  #define bytes_to_u32(buffer) (*(buffer + 3) | *(buffer + 2) << 8 | *(buffer + 1) << 16 | *(buffer) << 24)
#endif

static inline u8 u32_to_str(u32 x, u8 bytes[10])
{
  if ( x == 0 ) return 0;
  unsigned i = 0;
  while( x != 0 ){
    bytes[i++] = x % 10 + '0';
    x /= 10;
  }
  return i;

}

long decode(u8* qoi_buffer, u8** p6_buffer ){
  if (!
    (qoi_buffer[0] == 'q' && qoi_buffer[1] == 'o'
    && qoi_buffer[2] == 'i' && qoi_buffer[3] == 'f')
  ){
    error("Input file format does not cotain the QOI file format header according to the spec and thus might either be corrupted or follow another format");
  }
  u32 width = bytes_to_u32(qoi_buffer+4);
  u32 height = bytes_to_u32(qoi_buffer+8);
  u8 widths[10] = {0}, heights[10] = {0};
  u8 len_widths = u32_to_str(width, widths);
  u8 len_heights = u32_to_str(height, heights);
  *p6_buffer = *p6_buffer == NULL ? malloc(9 + len_heights + len_widths + 3 * width * height) : *p6_buffer;
  plog("p6_buffer size: %d", 9 + len_widths + len_heights);

  (*p6_buffer)[0] = 'P';
  (*p6_buffer)[1] = '6';
  (*p6_buffer)[2] = '\n';

  for(unsigned i = 0; i < len_widths; i++){
    (*p6_buffer)[3 + i] = widths[len_widths - 1 - i] ;  
  }
  (*p6_buffer)[3 + len_widths] = ' ';
  for(unsigned i = 0; i < len_heights; i++){
    (*p6_buffer)[4 + len_widths + i] = heights[len_heights - 1 - i] ;  
  }
  (*p6_buffer)[4 + len_widths + len_heights] = '\n'; 
  (*p6_buffer)[5 + len_widths + len_heights] = '2';
  (*p6_buffer)[6 + len_widths + len_heights] = '5';
  (*p6_buffer)[7 + len_widths + len_heights] = '5';
  (*p6_buffer)[8 + len_widths + len_heights] = '\n';

  u64 p6_cursor = 9 + len_heights + len_widths;
  u64 qoi_cursor = 14;
  
  struct qoi_pixel array[64] = {0};
  struct qoi_pixel qoi_pix = {0};
  struct p6_pixel prev = {0};
  struct p6_pixel curr = {0};
  i8 vardr, vardb, vardg, dr_dg, db_dg;
  for(u64 i = 0; i < width * height; i++){
    // QOI_OP_INDEX
    if ( (qoi_buffer[qoi_cursor] & 0xC0) == 0 )  {
      qoi_pix = array[qoi_buffer[qoi_cursor]];
      (*p6_buffer)[p6_cursor] = qoi_pix.r;
      (*p6_buffer)[p6_cursor + 1] = qoi_pix.g;
      (*p6_buffer)[p6_cursor + 2] = qoi_pix.b;
      p6_cursor += 3;
      qoi_cursor ++;
      prev = (struct p6_pixel){qoi_pix.r, qoi_pix.g, qoi_pix.b};
      plog("INDEX -> curr: r: %d, g: %d, b: %d", prev.r, prev.g, prev.b);
      continue;
    }
    // QOI_OP_DIFF
    // cur - prev = var -> curr = var + prev
    if ( (qoi_buffer[qoi_cursor] & 0xC0 ) == 0x40){
      vardr = ((qoi_buffer[qoi_cursor] >> 4) & 0x03 ) - 2; 
      vardg = ((qoi_buffer[qoi_cursor] >> 2) & 0x03 )- 2; 
      vardb = (qoi_buffer[qoi_cursor] & 0x03 ) - 2; 
      curr = (struct p6_pixel){prev.r + vardr, prev.g + vardg, prev.b + vardb};
      (*p6_buffer)[p6_cursor] = curr.r;
      (*p6_buffer)[p6_cursor + 1] = curr.g;
      (*p6_buffer)[p6_cursor + 2] = curr.b;
      array[hash(curr)] = (struct qoi_pixel){curr.r, curr.g, curr.b, 255};
      prev = curr;
      plog("DIFF -> curr: r: %d, g: %d, b: %d", prev.r, prev.g, prev.b);
      p6_cursor += 3;
      qoi_cursor ++;
      continue;
    }
    // QOI_OP_LUMA
    // dg = curr.g  - prev.g      => curr.g = dg + prev.g
    // dr_dg = curr.r - prev.r - curr.g + prev.g
    //       = curr.r - prev.r - dg - prev.g + prev.g
    //       = curr.r - prev.r - dg 
    // curr.r = dr_dg + prev.r + dg
    if ( (qoi_buffer[qoi_cursor] & 0xC0) == 0x80 ){
      vardg = (qoi_buffer[qoi_cursor] & 0x3F) - 32;
      dr_dg = (qoi_buffer[qoi_cursor + 1] >> 4) - 8;
      db_dg = (qoi_buffer[qoi_cursor + 1] & 0x0F) - 8;
      curr = (struct p6_pixel){dr_dg + prev.r + vardg, vardg + prev.g, db_dg + prev.b + vardg};
      (*p6_buffer)[p6_cursor] = curr.r;
      (*p6_buffer)[p6_cursor + 1] = curr.g;
      (*p6_buffer)[p6_cursor + 2] = curr.b;
      array[hash(curr)] = (struct qoi_pixel){curr.r, curr.g, curr.b, 255};
      prev = curr;
      plog("LUMA -> curr: r: %d, g: %d, b: %d", prev.r, prev.g, prev.b);
      p6_cursor += 3;
      qoi_cursor += 2;
      continue;
    }
    // QOI_OP_RGB
    if  (qoi_buffer[qoi_cursor] == 0xFE ){
      curr = (struct p6_pixel){qoi_buffer[qoi_cursor + 1], qoi_buffer[qoi_cursor + 2], qoi_buffer[qoi_cursor + 3]};
      (*p6_buffer)[p6_cursor] = curr.r;
      (*p6_buffer)[p6_cursor + 1] = curr.g;
      (*p6_buffer)[p6_cursor + 2] = curr.b; 
      array[hash(curr)] = (struct qoi_pixel){curr.r, curr.g, curr.b, 255};
      prev = curr;
      plog("RGB -> curr: r: %d, g: %d, b: %d", curr.r, curr.g, curr.b);
      p6_cursor += 3;
      qoi_cursor += 4;
      continue;
    }
    // QOI_OP_RGBA (impossible in the case of P6)
    if  (unlikely(qoi_buffer[qoi_cursor] == 0xFF )){
      error("an encoding of P6 format in QOI format is not supposed to encode QOI_OP_RGBA chunks!");
      continue;
    }
    // QOI_OP_RUN
    u8 run = (qoi_buffer[qoi_cursor] & 0x3F) + 1;
    unsigned j = 0;
    for(; j < run * 3; j+=3){
      (*p6_buffer)[p6_cursor+j] = curr.r;
      (*p6_buffer)[p6_cursor+j+1] = curr.g;
      (*p6_buffer)[p6_cursor+j+2] = curr.b;
      plog("RUN -> curr: r: %d, g: %d, b: %d", prev.r, prev.g, prev.b);
    }
    p6_cursor += j;
    qoi_cursor ++;
    i += run - 1;

  }
  if (!
    (qoi_buffer[qoi_cursor] == 0 &&
    qoi_buffer[qoi_cursor+1] == 0 && 
    qoi_buffer[qoi_cursor+2] == 0 && 
    qoi_buffer[qoi_cursor+3] == 0 && 
    qoi_buffer[qoi_cursor+4] == 0 && 
    qoi_buffer[qoi_cursor+5] == 0 && 
    qoi_buffer[qoi_cursor+6] == 0 && 
    qoi_buffer[qoi_cursor+7] == 1 )
  ){
    error("This file does not end with the proper end marker of a qoi file according to the QOI spec!");
  }


  return 9 + len_heights + len_widths + 3 * width * height;
}
