#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "error.h"
#include "header.h"
#include "parse.h"
#include "preprocess.h"
#include "tokenize.h"

char *user_input;
char *filename;
char *dir_name;
char *regs1[6] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
char *regs4[6] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
char *regs8[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

char *read_file(char *path)
{
  FILE *fp = fopen(path, "r");
  if (!fp)
    error("cannot open %s: %s", path, strerror(errno));

  if (fseek(fp, 0, SEEK_END) == -1)
    error("%s: fseek: %s", path, strerror(errno));
  size_t size = ftell(fp);
  if (fseek(fp, 0, SEEK_SET) == -1)
    error("%s: fseek: %s", path, strerror(errno));

  char *buf = calloc(1, size + 2);
  fread(buf, size, 1, fp);

  if (size == 0 || buf[size - 1] != '\n')
    buf[size++] = '\n';
  buf[size] = '\0';
  fclose(fp);
  return buf;
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "invalid argument");
    return 1;
  }

  filename = argv[1];

  int dir;
  for (dir = strlen(filename) - 1; dir >= 0; dir--)
  {
    if (filename[dir] == '/')
      break;
  }

  dir_name = calloc(1, dir + 1);
  if (dir != -1)
    strncpy(dir_name, filename, dir);
  else
    dir_name[0] = '.';

  user_input = read_file(filename);
  token = tokenize(user_input, true);

  token = preprocess(token);

  printf(".intel_syntax noprefix\n");

  while (!at_eof())
  {
    External *ext = external();

    switch (ext->kind)
    {
    case EXT_FUNC:
      for (StringLiteral *l = ext->literals; l; l = l->next)
      {
        printf(".data\n");
        printf(".LC%d:\n", l->offset);
        printf("  .string \"%.*s\"\n", l->str->len, l->str->ptr);
      }
      printf(".globl %.*s\n", ext->name->len, ext->name->ptr);

      printf(".text\n");
      printf("%.*s:\n", ext->name->len, ext->name->ptr);
      printf("  push rbp\n");
      printf("  mov rbp, rsp\n");
      printf("  sub rsp, %d\n", ext->stack_size);

      for (int i = 0; i < 6 && ext->offsets[i]; i++)
      {
        char *reg;
        if (i == 0)
        {
          if (ext->offsets[i] == 8)
            reg = regs8[i];
          else if (ext->offsets[i] == 4)
            reg = regs4[i];
          else if (ext->offsets[i] == 1)
            reg = regs1[i];
          else
            error("not implemented: offset %d", ext->offsets[i]);
        }
        else
        {
          if (ext->offsets[i] - ext->offsets[i - 1] == 8)
            reg = regs8[i];
          else if (ext->offsets[i] - ext->offsets[i - 1] == 4)
            reg = regs4[i];
          else if (ext->offsets[i] - ext->offsets[i - 1] == 1)
            reg = regs1[i];
          else
            error("not implemented: offset %d", ext->offsets[i] - ext->offsets[i - 1]);
        }
        printf("  mov [rbp - %d], %s\n", ext->offsets[i], reg);
      }

      for (int i = 0; ext->code[i]; i++)
      {
        gen(ext->code[i]);
        printf("  pop rax\n");
      }

      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      break;
    case EXT_GVAR:
      printf(".data\n");
      printf("%.*s:\n", ext->name->len, ext->name->ptr);
      printf("  .zero %d\n", ext->size);
      break;
    case EXT_ENUM:
    case EXT_STRUCT:
    case EXT_FUNCDECL:
    case EXT_TYPEDEF:
      break;
    }
  }

  return 0;
}
