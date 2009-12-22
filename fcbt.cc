#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/time.h>
#include <ctype.h>

static const char *num2card(int c) {
    static char b[3] = "qq";
    static const char *suit = "CDHS";
    static const char *rank = "A23456789TJQK";
    b[0] = rank[c % 13];
    b[1] = suit[c / 13];
    return b;
}

static int card2num(const char *c) {
    int rank = -1;
    switch (c[0]) {
	case 'A': rank =  0; break;
	case '2': rank =  1; break;
	case '3': rank =  2; break;
	case '4': rank =  3; break;
	case '5': rank =  4; break;
	case '6': rank =  5; break;
	case '7': rank =  6; break;
	case '8': rank =  7; break;
	case '9': rank =  8; break;
	case 'T': rank =  9; break;
	case 'J': rank = 10; break;
	case 'Q': rank = 11; break;
	case 'K': rank = 12; break;
    }
    int suit = -1;
    switch (c[1]) {
	case 'C': suit = 0; break;
	case 'D': suit = 1; break;
	case 'H': suit = 2; break;
	case 'S': suit = 3; break;
    }
    if (rank == -1 || suit == -1)
	return -1;
    return suit * 13 + rank;
}

static int timestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((tv.tv_sec) % 1000000) * 1000 + (tv.tv_usec / 1000);
}

static void exit_with_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(stderr, fmt, args);
    va_end(args);
    exit(1);
}

struct game {
    char col[8][52];
    char fc[4];
    char disc[4];
    char last;
    int ts;
    game() {
	init();
    }
    void init() {
	for (int i = 0; i < 8; i++)
	    for (int j = 0; j < 52; j++)
		col[i][j] = -1;
	for (int i = 0; i < 4; i++) {
	    fc[i] = -1;
	    disc[i] = -1;
	}
	last = -1;
	ts = 0;
    }
    void read() {
	char buf[1024];
	init();
	int colindex[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	while (fgets(buf, 1024, stdin) != NULL) {
	    while (strlen(buf) > 0 && isspace(buf[strlen(buf) - 1]))
		buf[strlen(buf) - 1] = 0;
	    if (strlen(buf) < 3 || buf[2] != ':')
		exit_with_error("Bad input line: %s\n", buf);
	    if (buf[0] == 'c') {
		// column
		int i = buf[1] - '0';
		if (i < 0 || i > 7)
		    exit_with_error("Bad input line: %s\n", buf);
		for (char *card = strtok(buf + 3, " "); card != NULL; card = strtok(NULL, " ")) {
		    int c = card2num(card);
		    if (c == -1)
			exit_with_error("Bad input line: %s\n", buf);
		    col[i][colindex[i]++] = c;
		}
	    } else if (buf[0] == 'f') {
		// free cell
		int i = buf[1] - '0';
		if (i < 0 || i > 3)
		    exit_with_error("Bad input line: %s\n", buf);
		char *card = strtok(buf + 3, " ");
		if (card != NULL) {
		    int c = card2num(card);
		    if (c == -1)
			exit_with_error("Bad input line: %s\n", buf);
		    col[i][colindex[i]++] = c;
		}
	    } else {
		exit_with_error("Bad input line: %s\n", buf);
	    }
	}
    }
    void write() {
	for (int i = 0; i < 8; i++) {
	    printf("c%d:", i);
	    for (int j = 0; j < 52 && col[i][j] != -1)
		printf(" %s", num2card(col[i][j]));
	    printf("\n");
	}
	for (int i = 0; i < 4; i++)
	    printf("f%d: %s\n", fc[i] == -1 ? "" : num2card(fc[i]));
    }
}

int main() {
    game g;
    g.read();
    g.write();
    return 0;
}
