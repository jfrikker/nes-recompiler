#include <stdint.h>
#include <stdio.h>
#include <string.h>

const char *NES_IDENTIFIER = "NES\x1a";

typedef struct {
  char identifier[4];
  uint8_t prgRomSize;
  uint8_t chrRomSize;
  uint16_t flags1;
  uint8_t ramSize;
  uint16_t flags2;
  char zero[5];
} NESHeader;

int isValidHeader(NESHeader *header) {
  return !strncmp(header->identifier, NES_IDENTIFIER, 4);
}

int hasHorizontalMirroring(NESHeader *header) {
  return (header->flags1 & 0x0009) ^ 0x0009;
}

int hasVerticalMirroring(NESHeader *header) {
  return (header->flags1 & 0x0009) ^ 0x0008;
}

const char *getBooleanString(int flag) {
  return flag ? "true" : "false";
}

void printHeader(NESHeader *header) {
  printf("PRG ROM size: %d x 16K\n", header->prgRomSize);
  printf("CHR ROM size: %d x 8K\n", header->chrRomSize);
  printf("Horizontal mirroring: %s\n", getBooleanString(hasHorizontalMirroring(header)));
  printf("Vertical mirroring: %s\n", getBooleanString(hasVerticalMirroring(header)));
}

int main(int argc, char **argv) {
  NESHeader header;
  fread(&header, sizeof(NESHeader), 1, stdin);

  if (!isValidHeader(&header)) {
    fprintf(stderr, "Not a valid header file\n");
    return 1;
  }

  printHeader(&header);
}
