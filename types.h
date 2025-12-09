#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

struct qoi_header {
 char magic[4]; // magic bytes "qoif"
 uint32_t width; // image width in pixels (BE)
 uint32_t height; // image height in pixels (BE)
 uint8_t channels; // 3 = RGB, 4 = RGBA
 uint8_t colorspace; // 0 = sRGB with linear alpha
                     // 1 = all channels linear
}__attribute__((packed));

struct QOI_OP_RGB{
  u8 tag;
  u8 r;
  u8 g;
  u8 b;
}__attribute__((packed));

static inline u8 rgb_to_bytes1(struct QOI_OP_RGB p) {
  return p.tag;
}
static inline u8 rgb_to_bytes2(struct QOI_OP_RGB p) {
  return p.r;
}
static inline u8 rgb_to_bytes3(struct QOI_OP_RGB p) {
  return p.g;
}
static inline u8 rgb_to_bytes4(struct QOI_OP_RGB p) {
  return p.b;
}

struct QOI_OP_RGBA{
  u8 tag;
  u8 r;
  u8 g;
  u8 b;
  u8 a;
}__attribute__((packed));

static inline u8 rgba_to_bytes1(struct QOI_OP_RGBA p){
  return p.tag;
}
static inline u8 rgba_to_bytes2(struct QOI_OP_RGBA p){
  return p.r;
}
static inline u8 rgba_to_bytes3(struct QOI_OP_RGBA p){
  return p.g;
}
static inline u8 rgba_to_bytes4(struct QOI_OP_RGBA p){
  return p.b;
}
static inline u8 rgba_to_bytes5(struct QOI_OP_RGBA p){
  return p.a;
}
struct QOI_OP_INDEX {
  u8 tag: 2;
  u8 index: 6;
} __attribute__((packed));

static inline u8 idx_to_bytes(struct QOI_OP_INDEX p){
  return p.tag << 6 | p.index;
}

struct QOI_OP_DIFF {
  u8 tag: 2;
  u8 dr : 2;
  u8 dg : 2;
  u8 db : 2;
} __attribute__((packed));

static inline u8 diff_to_bytes(struct QOI_OP_DIFF p){
  return p.tag << 6 | p.dr << 4 | p.dg << 2 | p.db;
}

struct QOI_OP_LUMA {
  u8 tag: 2;
  u8 diff_green: 6;
  u8 dr_dg: 4;
  u8 db_dg: 4;
} __attribute__((packed));

static inline u8 luma_to_bytes_higher(struct QOI_OP_LUMA p){
  return p.tag << 6 | p.diff_green;
}
static inline u8 luma_to_bytes_lower(struct QOI_OP_LUMA p){
  return p.dr_dg << 4 | p.db_dg;
}

struct QOI_OP_RUN {
  u8 tag: 2;
  u8 run: 6;
} __attribute__((packed));

static inline u8 run_to_bytes(struct QOI_OP_RUN p){
  return p.tag << 6 | p.run;
}


struct p6_pixel {
  u8 r;
  u8 g;
  u8 b;
};

struct qoi_pixel {
  u8 r;
  u8 g;
  u8 b;
  u8 a;
};

#endif
