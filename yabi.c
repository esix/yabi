#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// linux?
#include <termios.h>
#include <unistd.h>

#define DATA_SIZE 30000

static char data_highway[DATA_SIZE];
static char *p;

int getch() {
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



int do_command(char command, FILE *input)
{
	char c;
	long pos;

	switch(command) {
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
		//*p = getchar();
		*p = getch();
		break;
	case '[':
		pos = ftell(input);

		// TODO: check *p on zero here and skip the body

		while((*p) != 0) {
			fseek(input, pos, SEEK_SET);
			c = getc(input);
			while( c!=']' && c!=EOF) {
				do_command(c,input);
				c = getc(input);
			}

		}

	}

}

int main(int argc, char **argv) {

	char command;
	FILE *input;

	if (argc > 1) {
	  if(!strcmp(argv[1],"-")) {
	    input = stdin;
	  } else {
	    input = fopen(argv[1],"r");
	    if (NULL==input) {
	      fprintf(stderr, "Unable to open \"%s\"\n", argv[1]);
	      exit(EXIT_FAILURE);
	    }
	  }
	} else {
  		input = stdin;
	}

	p = &data_highway[0];

	while((command = getc(input)) != EOF) {
		do_command(command, input);
	}

	return 0;
}
