
#include <argp.h>
#include <pretty.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "decode.h"
#include "encode.h"
#include "viewer.h"

enum command_type { CMD_NONE, CMD_ENCODE, CMD_DECODE, CMD_DISPLAY };

enum display_format {
  DISPLAY_PPM_P6,
  DISPLAY_QOI,
  DISPLAY_AUTO // Auto-detect format
};

struct arguments {
  enum command_type cmd;
  char *input;
  char *output;
  enum display_format display_fmt;
};

static char doc[] = "qoi-tool -- encode and decode QOI images";

static char args_doc[] = "encode|decode";

static struct argp_option options[] = {
    {"input", 'i', "FILE", 0, "Input file (required)", 0},
    {"output", 'o', "FILE", 0, "Output file (optional, default stdout)", 0},
    {"ppm", 0, 0, OPTION_ALIAS, 0, 0},
    {"qoi", 0, 0, OPTION_ALIAS, 0, 0},
    {0}};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key) {

  case ARGP_KEY_ARG:
    if (strcmp(arg, "encode") == 0)
      arguments->cmd = CMD_ENCODE;
    else if (strcmp(arg, "decode") == 0)
      arguments->cmd = CMD_DECODE;
    else if (strcmp(arg, "display") == 0)
      arguments->cmd = CMD_DISPLAY;
    else
      argp_usage(state);
    break;

  case 'i':
    arguments->input = arg;
    break;

  case 'o':
    arguments->output = arg;
    break;

  case 'f':
    if (strcmp(arg, "p6") == 0 || strcmp(arg, "ppm") == 0)
      arguments->display_fmt = DISPLAY_PPM_P6;
    else if (strcmp(arg, "qoi") == 0)
      arguments->display_fmt = DISPLAY_QOI;
    else if (strcmp(arg, "auto") == 0)
      arguments->display_fmt = DISPLAY_AUTO;
    else
      argp_error(state, "Invalid format. Use: p6, qoi, or auto");
    break;

  case ARGP_KEY_END:
    if (arguments->cmd == CMD_NONE)
      argp_error(state, "Missing subcommand: encode|decode|display");

    if (!arguments->input)
      argp_error(state, "Missing required -i/--input FILE");

    if (!arguments->display_fmt)
      arguments->display_fmt = DISPLAY_AUTO;

    // Set default display format if not specified
    if (arguments->cmd == CMD_DISPLAY &&
        arguments->display_fmt == DISPLAY_AUTO) {
      // Auto-detect based on file extension
      const char *ext = strrchr(arguments->input, '.');
      if (ext) {
        if (strcmp(ext, ".qoi") == 0)
          arguments->display_fmt = DISPLAY_QOI;
        else if (strcmp(ext, ".ppm") == 0 || strcmp(ext, ".p6") == 0)
          arguments->display_fmt = DISPLAY_PPM_P6;
      }
    }
    break;

  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, 0, NULL, NULL};

void cli(int argc, char **argv) {
  struct arguments args;
  args.cmd = CMD_NONE;
  args.input = NULL;
  args.output = NULL;

  argp_parse(&argp, argc, argv, 0, 0, &args);

  /* READ INPUT FILE */
  FILE *in = fopen(args.input, "rb");
  if (!in) {
    fprintf(stderr, "Failed to open input file: %s\n", args.input);
    exit(1);
  }
  fseek(in, 0, SEEK_END);
  long size = ftell(in);
  fseek(in, 0, SEEK_SET);

  unsigned char *buffer = malloc(size);
  fread(buffer, 1, size, in);
  fclose(in);

  /* Decide OUTPUT target */
  FILE *out = stdout;
  if (args.output) {
    out = fopen(args.output, "wb");
    if (!out) {
      fprintf(stderr, "Failed to open output file: %s\n", args.output);
      exit(1);
    }
  }

  if (args.cmd == CMD_ENCODE) {

    u8 *encoded = NULL;
    int out_len;
    out_len = encode(buffer, size, &encoded);

    fwrite(encoded, 1, out_len, out);

    free(encoded);

  } else if (args.cmd == CMD_DECODE) {

    u8 *decoded = NULL;
    int out_len;
    out_len = decode(buffer, &decoded);

    fwrite(decoded, 1, out_len, out);

    free(decoded);
  } else if (args.cmd == CMD_DISPLAY) {
    switch(args.display_fmt){
      case DISPLAY_PPM_P6:
        display_ppm_p6(buffer);
        break;
      case DISPLAY_QOI:
        display_qoi(buffer);
        break;
    }
  }

  if (args.output)
    fclose(out);

  free(buffer);
}
