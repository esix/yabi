#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <termios.h>
#include <unistd.h>

#define DATA_SIZE 30000

#define OUT_OF_MEMORY "Out of memory!"
#define ERROR_NO_CLOSING_BRACKET 222


/* program char */
typedef char pchar_t;

/* memory */
typedef unsigned char mchar_t;


static mchar_t data_highway[DATA_SIZE];
static mchar_t *mem;

static pchar_t *g_program;

int
getch(void);

static pchar_t *
read_program (FILE *, size_t *);

int
do_program (pchar_t const *, pchar_t const *);

pchar_t const *
next_op(pchar_t const *start, pchar_t const *end);


int
getch(void)
{
  return getchar();
}


static pchar_t *
read_program (FILE *input, size_t *out_program_size)
{
  pchar_t *program;
  size_t buffer_size;           /* number of program-characters in allocated buffer */
  size_t program_size;          /* number of program-characters in program          */
  size_t n;

  buffer_size = BUFSIZ;
  program_size = 0;

  program = malloc (buffer_size * sizeof (pchar_t));
  if (!program)
    {
      fprintf (stderr, OUT_OF_MEMORY);
      return NULL;
    }

  do
    {
      /* ensure buffer is large enough to read BUFSIZ program-characters */
      if (program_size + BUFSIZ > buffer_size)
        {
          buffer_size *= 2;
          program = realloc (program, buffer_size * sizeof (pchar_t));
          if (!program)
            {
              fprintf (stderr, OUT_OF_MEMORY);
              return NULL;
            }
        }

      n = fread (program + program_size, sizeof (pchar_t), BUFSIZ, input);
      program_size += n;
    }
  while(n == BUFSIZ);

  if (out_program_size != NULL)
    *out_program_size = program_size;

  return program;
}


pchar_t const *
next_op(pchar_t const *start, pchar_t const *end)
{
  int num_brackets = 0;
  pchar_t const *p = start;

  do
    {
      if (*p == '[')
        ++num_brackets;
      else if (*p == ']')
        --num_brackets;

      ++p;
    }
  while (p < end && num_brackets);

  if (p == end && num_brackets)
    return NULL;
  else
    return p;
}


int
do_program (pchar_t const *start, pchar_t const *end)
{
  pchar_t const *prg = start;

  while (prg < end)
    {
      switch(*prg)
        {
        case '>':
          ++mem;
          break;

        case '<':
          --mem;
          break;

        case '+':
          ++*mem;
          break;

        case '-':
          --*mem;
          break;

        case '.':
          putchar(*mem);
          break;

        case ',':
          /* *p = getchar(); */
          *mem = getch();
          break;

        case '[':
          {
            pchar_t const * block_end = next_op(prg, end);
            
            if (block_end == NULL)
              return ERROR_NO_CLOSING_BRACKET;

            while (*mem)
              {
                int result = do_program (prg + 1, block_end - 1);
                if (result != 0)
                  return result;
              }
            prg = block_end - 1;
          }
          break;
      }

      ++prg;
    }
  return 0;
}


int
main(int argc, char **argv)
{
  size_t program_size;
  FILE *input = stdin;

  if (argc > 1)
    {
      if (!strcmp(argv[1],"-"))
        input = stdin;
      else
        {
          input = fopen(argv[1],"r");
          if (input == NULL)
            {
              fprintf(stderr, "Unable to open \"%s\"\n", argv[1]);
              exit(EXIT_FAILURE);
            }
        }
    } else {
      input = stdin;
    }

  g_program = read_program (input, &program_size);
  if (!g_program)
    return ENOMEM;

  struct termios options;
  tcgetattr (STDIN_FILENO, &options);
  options.c_lflag &= ~( ICANON | ECHO );
  tcsetattr (STDIN_FILENO, TCSANOW, &options);
  
  mem = &data_highway[0];

  do_program (g_program, g_program + program_size);

  return 0;
}
