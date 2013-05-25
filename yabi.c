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
static mchar_t *p;

static pchar_t *g_program;

int
getch(void);


static pchar_t *
read_program (FILE *, size_t *);

int
do_program (pchar_t const *, size_t);


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


int
do_program (pchar_t const *program, size_t program_len)
{
  pchar_t const *prg;
  pchar_t const *prg_end;

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
            pchar_t const * closing_bracket = prg;
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
  
  printf("OKAY, Program is being read ok [%d]\n", program_size);

  p = &data_highway[0];

  do_program (g_program, program_size);

  return 0;
}
