#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <termios.h>
#include <unistd.h>

#define DATA_SIZE 30000

#define OUT_OF_MEMORY "Out of memory!"
#define ERROR_NO_CLOSING_BRACKET 222

static unsigned char data_highway[DATA_SIZE];
static unsigned char *p;
static unsigned char *g_program;

int
getch(void);


static unsigned char *
read_program (FILE *, size_t *);

int
do_program (unsigned char const *, size_t);


int
getch(void)
{
  struct termios oldt, newt;
  int ch;
  tcgetattr( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &newt );
  ch = getchar();
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
  return ch;
}

static unsigned char *
read_program (FILE *input, size_t *out_program_size)
{
  unsigned char *program;
  size_t buffer_size;
  size_t program_size;
  size_t n;

  buffer_size = BUFSIZ;
  program_size = 0;

  program = malloc (buffer_size);
  if (!program)
    {
      fprintf (stderr, OUT_OF_MEMORY);
      return NULL;
    }

  while ((n = fread (program + program_size, 1, BUFSIZ, input)) > 0)
    {
      program_size += n;
      if (program_size + BUFSIZ > buffer_size)
        {
          buffer_size *= 2;
          program = realloc (program, buffer_size);
          if (!program)
            {
              fprintf (stderr, OUT_OF_MEMORY);
              return NULL;
            }
        }
    }

  *out_program_size = program_size;
  return program;
}


int
do_program (unsigned char const *program, size_t program_len)
{
  unsigned char const *prg;
  unsigned char const *prg_end;

  prg = program;
  prg_end = program + program_len;

  while (prg < prg_end)
    {
      switch(*prg)
        {
        case '>':
          ++p;
          break;

        case '<':
          --p;
          break;

        case '+':
          ++*p;
          break;

        case '-':
          --*p;
          break;

        case '.':
          putchar(*p);
          break;

        case ',':
          /* *p = getchar(); */
          *p = getch();
          break;

        case '[':
          {
            unsigned char const * closing_bracket = prg;
            int num_brackets = 0;

            do
              {
                if (*closing_bracket == '[')
                  ++num_brackets;
                else if (*closing_bracket == ']')
                  --num_brackets;

                 ++closing_bracket;
               }
            while (closing_bracket < prg_end && num_brackets > 0);

            if (closing_bracket == prg_end)
              return ERROR_NO_CLOSING_BRACKET;

            while (*p == 0)
              {
                int result = do_program (prg + 1, closing_bracket - prg);
                if (result != 0)
		  return result;
              }
            prg = closing_bracket;
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

  p = &data_highway[0];

  do_program (g_program, program_size);

  return 0;
}
