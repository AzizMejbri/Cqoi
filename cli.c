
#include <argp.h>
#include <pretty.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "encode.h"
#include "decode.h"

enum command_type { CMD_NONE, CMD_ENCODE, CMD_DECODE };

struct arguments {
  enum command_type cmd;
  char *input;
  char *output;
};

static char doc[] = "qoi-tool -- encode and decode QOI images";

static char args_doc[] = "encode|decode";

static struct argp_option options[] = {
    {"input", 'i', "FILE", 0, "Input file (required)", 0},
    {"output", 'o', "FILE", 0, "Output file (optional, default stdout)", 0},
    {0}};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key) {

  case ARGP_KEY_ARG:
    if (strcmp(arg, "encode") == 0)
      arguments->cmd = CMD_ENCODE;
    else if (strcmp(arg, "decode") == 0)
      arguments->cmd = CMD_DECODE;
    else
      argp_usage(state);
    break;

  case 'i':
    arguments->input = arg;
    break;

  case 'o':
    arguments->output = arg;
    break;

  case ARGP_KEY_END:
    if (arguments->cmd == CMD_NONE)
      argp_error(state, "Missing subcommand: encode|decode");

    if (!arguments->input)
      argp_error(state, "Missing required -i/--input FILE");
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

    u8* encoded = NULL;
    int out_len;
    out_len = encode(buffer, size, &encoded);

    fwrite(encoded, 1, out_len, out);

    free(encoded);

  } else if (args.cmd == CMD_DECODE) {

    u8* decoded = NULL;
    int out_len;
    out_len = decode(buffer, &decoded);

    fwrite(decoded, 1, out_len, out);

    free(decoded);
  }

  if (args.output)
    fclose(out);

  free(buffer);
}
