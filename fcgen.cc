#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

static const char *num2card(int c) {
    static char b[3] = "qq";
    static const char suit[] = "CDHS";
    static const char rank[] = "A23456789TJQK";
    b[0] = rank[c % 13];
    b[1] = suit[c / 13];
    return b;
}

int main() {
    int cards[52];
    bool dealtCards[52];
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int s = ((tv.tv_sec) % 1000) * 1000 + (tv.tv_usec / 1000);
    srandom(s);
    for (int i = 0; i < 52; i++)
	dealtCards[i] = false;
    for (int i = 0; i < 52; i++) {
	int c = 52 - i;
	int r = random() % c;
	int tc = 0;
	while (true) {
	    if (dealtCards[tc]) {
		tc++;
		continue;
	    }
	    if (r-- == 0) {
		cards[i] = tc;
		dealtCards[tc] = true;
		break;
	    } else
		tc++;
	}
    }
    int col = 0;
    for (int i = 0; i < 52; i++) {
	if (i == 0 || i == 7 || i == 14 || i == 21 || i == 28 || i == 34 || i == 40 || i == 46)
	    printf("%sc%d:", i == 0 ? "" : "\n", col++);
	printf(" %s", num2card(cards[i]));
    }
    printf("\n");
    for (int i = 0; i < 4; i++)
	printf("f%d:\n", i);
    for (int i = 0; i < 4; i++)
	printf("d%d:\n", i);
}
