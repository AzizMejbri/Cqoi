#include "encode.h"

#include <assert.h>
#include <pretty.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

#define between(value, a, b) ((i64)a <= (i64)value && (i64)value <= (i64)b)
#define diffr_g(p1, p2) ((i16)p1.r - (i16)p2.r - (i16)p1.g + (i16)p2.g)
#define diffb_g(p1, p2) ((i16)p1.b - (i16)p2.b - (i16)p1.g + (i16)p2.g)
#define dr(p1, p2) ((i16)p1.r - (i16)p2.r)
#define dg(p1, p2) ((i16)p1.g - (i16)p2.g)
#define db(p1, p2) ((i16)p1.b - (i16)p2.b)
#define hash(p)                                                                \
  (((u32)p.r * 3 + (u32)p.g * 5 + (u32)p.b * 7 + (u32)p.a * 11) & 63)
#define eq_p6(p1, p2) (p1.r == p2.r && p1.g == p2.g && p1.b == p2.b)
#define eq_qoi(p1, p2)                                                         \
  (p1.r == p2.r && p1.g == p2.g && p1.b == p2.b && p1.a == p2.a)
#define from_p6_pixel(p) ((struct qoi_pixel){p.r, p.g, p.b, 255})
#define flush fflust(stdout)

struct p6_pixel from_qoi_pixel(struct qoi_pixel pixel) {
  return (struct p6_pixel){pixel.r, pixel.g, pixel.b};
}

long encode(u8 *p6_buffer, u32 p6_size, u8 **qoi_buffer) {
  assert(p6_buffer[0] == 'P' && p6_buffer[1] == '6' && p6_buffer[2] == '\n');
  info("p6_buffer size: %u", p6_size);
  u64 i = 3;
  u32 width = 0, height = 0, max_col_val = 0;
  while (p6_buffer[i] != ' ') {
    width = width * 10 + p6_buffer[i] - '0';
    i++;
  }
  i++;
  while (p6_buffer[i] != '\n') {
    height = height * 10 + p6_buffer[i] - '0';
    i++;
  }
  i++;
  while (p6_buffer[i] != '\n') {
    max_col_val = max_col_val * 10 + p6_buffer[i] - '0';
    i++;
  }
  i++;
  struct p6_pixel p6_pixel_vec[width * height];

  u32 total_pixels = width * height;
  u64 byte_offset;
  for (u32 pixel_idx = 0; pixel_idx < total_pixels; pixel_idx++) {
    byte_offset = i + pixel_idx * 3;
    p6_pixel_vec[pixel_idx] =
        (struct p6_pixel){.r = p6_buffer[byte_offset],
                          .g = p6_buffer[byte_offset + 1],
                          .b = p6_buffer[byte_offset + 2]};
  }
  *qoi_buffer = *qoi_buffer == NULL
                    ? malloc(sizeof(struct qoi_header) + 5 * width * height + 11)
                    : *qoi_buffer;
  memcpy(*qoi_buffer,
         &(struct qoi_header){.magic = {'q', 'o', 'i', 'f'},
                              .width = width,
                              .height = height,
                              .channels = 3,
                              .colorspace = 0},
         sizeof(struct qoi_header));
  struct qoi_pixel prev = (struct qoi_pixel){0, 0, 0, 255};
  struct qoi_pixel curr;
  struct qoi_pixel array[64] = {0};
  u8 h;
  i16 vardr, vardg, vardb;
  i8 dr_dg, db_dg;
  u64 j = 14, run = 0;
  // asm volatile("int3");
  for (i = 0; i < width * height; i++) {
    curr = from_p6_pixel(p6_pixel_vec[i]);

    // QOI_OP_RUN case
    if (eq_qoi(curr, prev)) {
      run = 1;
      // Look ahead for consecutive same pixels
      while (run < 62 && i + run < width * height &&
             eq_qoi(curr, from_p6_pixel(p6_pixel_vec[i + run]))) {
        run++;
      }
      (*qoi_buffer)[j++] = 0xC0 | (run - 1);
      i += (run - 1); // Skip the pixels we just encoded as run
      continue;
    }

    h = hash(curr);

    // QOI_OP_INDEX case
    if (eq_qoi(curr, array[h])) {
      (*qoi_buffer)[j++] = h; // Just the index (lower 6 bits)
      prev = curr;
      array[h] = curr;
      continue;
    }

    array[h] = curr;

    vardr = dr(curr, prev);
    vardg = dg(curr, prev);
    vardb = db(curr, prev);

    // QOI_OP_DIFF case
    if (between(vardr, -2, 1) && between(vardg, -2, 1) &&
        between(vardb, -2, 1)) {
      (*qoi_buffer)[j++] =
          0x40 | ((vardr + 2) << 4) | ((vardg + 2) << 2) | (vardb + 2);
      prev = curr;
      continue;
    }

    dr_dg = diffr_g(curr, prev);
    db_dg = diffb_g(curr, prev);

    // QOI_OP_LUMA case
    if (between(vardg, -32, 31) && between(dr_dg, -8, 7) &&
        between(db_dg, -8, 7)) {
      (*qoi_buffer)[j++] = 0x80 | (vardg + 32);
      (*qoi_buffer)[j++] = ((dr_dg + 8) << 4) | (db_dg + 8);
      prev = curr;
      continue;
    }

    // QOI_OP_RGB case
    (*qoi_buffer)[j++] = 0xFE;
    (*qoi_buffer)[j++] = curr.r;
    (*qoi_buffer)[j++] = curr.g;
    (*qoi_buffer)[j++] = curr.b;
    prev = curr;
  }
  

  // end marker
  (*qoi_buffer)[j] = 0;
  (*qoi_buffer)[j + 1] = 0;
  (*qoi_buffer)[j + 2] = 0;
  (*qoi_buffer)[j + 3] = 0;
  (*qoi_buffer)[j + 4] = 0;
  (*qoi_buffer)[j + 5] = 0;
  (*qoi_buffer)[j + 6] = 0;
  (*qoi_buffer)[j + 7] = 1;

  return j + 8;
}
