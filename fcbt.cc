#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>

struct game;

static int deadline;
static game *theGame;
static game *topGame = NULL;

#define SOLVED 0
#define PRUNED 1
#define TIMEOUT 2
#define EMPTY 255
#define NO_SUCH_CARD 255

static unsigned short crc16[] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

static const char *num2card(int c) {
    if (c < 0 || c >= 52)
	return "XX";
    static char b[3] = "qq";
    static const char *suit = "CDSH";
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
	case 'S': suit = 2; break;
	case 'H': suit = 3; break;
    }
    if (rank == -1 || suit == -1)
	return NO_SUCH_CARD;
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
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(1);
}

struct gameEntry {
    gameEntry *next;
    game *gameP;
};

static gameEntry *gameHash[65536];

struct game {
    unsigned char col[8][52];
    signed char colptr[8];
    unsigned char fc[4];
    unsigned char disc[4];
    unsigned char lastMovedCard;
    int ts;
    struct game *previous;
    int depth;

    game(FILE *is) {
	for (int i = 0; i < 8; i++) {
	    for (int j = 0; j < 52; j++)
		col[i][j] = EMPTY;
	    colptr[i] = -1;
	}
	for (int i = 0; i < 4; i++) {
	    fc[i] = EMPTY;
	    disc[i] = EMPTY;
	}
	lastMovedCard = EMPTY;
	char buf[1024];
	while (fgets(buf, 1024, is) != NULL) {
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
		    if (c == NO_SUCH_CARD)
			exit_with_error("Bad input line: %s\n", buf);
		    col[i][++colptr[i]] = c;
		}
	    } else if (buf[0] == 'f') {
		// free cell
		int i = buf[1] - '0';
		if (i < 0 || i > 3)
		    exit_with_error("Bad input line: %s\n", buf);
		char *card = strtok(buf + 3, " ");
		if (card != NULL) {
		    int c = card2num(card);
		    if (c == NO_SUCH_CARD)
			exit_with_error("Bad input line: %s\n", buf);
		    col[i][++colptr[i]] = c;
		}
	    } else {
		exit_with_error("Bad input line: %s\n", buf);
	    }
	}
	previous = NULL;
	depth = 0;
	ts = timestamp();
    }

    game(game *previous, int move) {
	for (int i = 0; i < 8; i++)
	    for (int j = 0; j < 52; j++) {
		int c = previous->col[i][j];
		col[i][j] = c;
		if (c == EMPTY)
		    break;
	    }
	for (int i = 0; i < 4; i++) {
	    fc[i] = previous->fc[i];
	    disc[i] = previous->disc[i];
	}

	// Moves are encoded as follows: source * 10 + destination
	// source = 0..3 (free cells) 4..11 (columns)
	// destination = 0 (free cell) 1 (discard) 2..9 (columns)
	// This gives move codes 0..119.
	int src = move / 10;
	int dst = move % 10;
	if (src < 4) {
	    // Move from free cell
	    // Note: move to another free cell won't happen
	    lastMovedCard = fc[src];
	    fc[src] = EMPTY;
	    if (dst == 1)
		// Move to discard
		disc[lastMovedCard / 13]++;
	    else
		// Move to column
		col[dst - 2][++colptr[dst - 2]] = lastMovedCard;
	} else {
	    // Move from column
	    src -= 4;
	    lastMovedCard = col[src][colptr[src]];
	    col[src][colptr[src]--] = EMPTY;;
	    if (dst == 0) {
		// Move to free cell
		for (int i = 0; i < 4; i++)
		    if (fc[i] == EMPTY) {
			fc[i] = lastMovedCard;
			break;
		    }
	    } else if (dst == 1) {
		// Move to discard
		disc[lastMovedCard / 13]++;
	    } else {
		// Move to another column
		col[dst - 2][++colptr[dst - 2]] = lastMovedCard;
	    }
	}

	this->previous = previous;
	depth = depth = previous->depth + 1;
	ts = timestamp();
    }

    void write() {
	for (int i = 0; i < 8; i++) {
	    printf("c%d:", i);
	    for (int j = 0; j < 52 && col[i][j] != EMPTY; j++)
		printf(" %s", num2card(col[i][j]));
	    printf("\n");
	}
	for (int i = 0; i < 4; i++)
	    printf("f%d: %s\n", i, fc[i] == EMPTY ? "" : num2card(fc[i]));
    }

    unsigned short crc() {
	unsigned short result = 0;

	unsigned char sorted_fc[4];
	for (int i = 0 ; i < 4; i++)
	    sorted_fc[i] = fc[i];
	for (int i = 0; i < 3; i++)
	    for (int j = i + 1; j < 4; j++)
		if (sorted_fc[i] > sorted_fc[j]) {
		    unsigned char t = sorted_fc[i];
		    sorted_fc[i] = sorted_fc[j];
		    sorted_fc[j] = t;
		}

	for (int i = 0; i < 4; i++)
	    result = result >> 8 ^ crc16[result & 0xFF ^ sorted_fc[i] & 0xFF];

	int sorted_index[8];
	for (int i = 0; i < 8; i++)
	    sorted_index[i] = i;
	for (int i = 0; i < 7; i++)
	    for (int j = i + 1; j < 8; j++)
		if (col[sorted_index[i]][0] > col[sorted_index[j]][0]) {
		    int t = sorted_index[i];
		    sorted_index[i] = sorted_index[j];
		    sorted_index[j] = t;
		}
	for (int i = 0; i < 8; i++)
	    for (int j = 0; j < 52; j++) {
		unsigned char c = col[sorted_index[i]][j];
		result = result >> 8 ^ crc16[result & 0xFF ^ c & 0xFF];
		if (c == EMPTY)
		    break;
	    }

	return result;
    }

    bool equals(const game *that) {
	unsigned char this_sorted_fc[4];
	unsigned char that_sorted_fc[4];
	for (int i = 0 ; i < 4; i++) {
	    this_sorted_fc[i] = fc[i];
	    that_sorted_fc[i] = that->fc[i];
	}
	for (int i = 0; i < 3; i++)
	    for (int j = i + 1; j < 4; j++) {
		if (this_sorted_fc[i] > this_sorted_fc[j]) {
		    unsigned char t = this_sorted_fc[i];
		    this_sorted_fc[i] = this_sorted_fc[j];
		    this_sorted_fc[j] = t;
		}
		if (that_sorted_fc[i] > that_sorted_fc[j]) {
		    unsigned char t = that_sorted_fc[i];
		    that_sorted_fc[i] = that_sorted_fc[j];
		    that_sorted_fc[j] = t;
		}
	    }
	for (int i = 0; i < 4; i++)
	    if (this_sorted_fc[i] != that_sorted_fc[i])
		return false;
	int this_sorted_index[8];
	int that_sorted_index[8];
	for (int i = 0; i < 8; i++) {
	    this_sorted_index[i] = i;
	    that_sorted_index[i] = i;
	}
	for (int i = 0; i < 7; i++)
	    for (int j = i + 1; j < 8; j++) {
		if (col[this_sorted_index[i]][0] > col[this_sorted_index[j]][0]) {
		    int t = this_sorted_index[i];
		    this_sorted_index[i] = this_sorted_index[j];
		    this_sorted_index[j] = t;
		}
		if (that->col[that_sorted_index[i]][0] > that->col[that_sorted_index[j]][0]) {
		    int t = that_sorted_index[i];
		    that_sorted_index[i] = that_sorted_index[j];
		    that_sorted_index[j] = t;
		}
	    }
	for (int i = 0; i < 8; i++)
	    for (int j = 0; j < 52; j++) {
		unsigned char thisc = col[this_sorted_index[i]][j];
		unsigned char thatc = that->col[that_sorted_index[i]][j];
		if (thisc != thatc)
		    return false;
		if (thisc == EMPTY)
		    continue;
	    }
	return true;
    }

    int solve() {
	// First, check if we're done
	bool done = true;
	for (int i = 0; i < 4; i++)
	    if (disc[i] != 12) {
		done = false;
		break;
	    }
	if (done) {
	    topGame = this;
	    return SOLVED;
	}

	// Next, check if it's too late
	int now = timestamp();
	if (now > deadline) {
	    topGame = this;
	    return TIMEOUT;
	}

	// Next, check whether or not we're original
	unsigned short ourCrc = crc();
	gameEntry *ge = gameHash[ourCrc];
	while (ge != NULL) {
	    if (ge->gameP->equals(this))
		// We're not original; abandon this branch
		return PRUNED;
	    ge = ge->next;
	}
	// We *are* original; add self to hashtable
	gameEntry *newGameEntry = new gameEntry;
	newGameEntry->next = gameHash[ourCrc];
	newGameEntry->gameP = this;
	gameHash[ourCrc] = newGameEntry;

	// First, try moving card from free cell
	for (int i = 0; i < 4; i++) {
	    unsigned char c = fc[i];
	    if (c == EMPTY)
		// No card in this free cell
		continue;
	    if (c == lastMovedCard)
		// We moved this card just before
		continue;
	    // Try moving this card to discard
	    int suit = c / 13;
	    int rank = c % 13;
	    if (((disc[suit] + 1) & 255) == rank) {
		game *g = new game(this, i * 10 + 1);
		int res = g->solve();
		if (res != PRUNED)
		    return res;
		else
		    delete g;
	    }
	    // Try moving this card to a column
	    bool triedMovingToEmptyColumn = false;
	    for (int j = 0; j < 8; j++) {
		int ptr = colptr[j];
		if (ptr == -1) {
		    // Destination is empty column
		    if (triedMovingToEmptyColumn)
			continue;
		    else {
			game *g = new game(this, i * 10 + j + 2);
			int res = g->solve();
			if (res != PRUNED)
			    return res;
			else {
			    delete g;
			    triedMovingToEmptyColumn = true;
			}
		    }
		} else {
		    // Destination is non-empty column
		    unsigned char topCard = col[j][colptr[j]];
		    int destSuit = topCard / 13;
		    int destRank = topCard % 13;
		    if (((suit ^ destSuit) & 1) == 0)
			continue;
		    if (rank != destRank + 1)
			continue;
		    game *g = new game(this, i * 10 + j + 2);
		    int res = g->solve();
		    if (res != PRUNED)
			return res;
		    else
			delete g;
		}
	    }
	}

	// Next, try moving card from a column
	for (int i = 0; i < 8; i++) {
	    signed char ptr = colptr[i];
	    if (ptr == -1)
		continue;
	    unsigned char c = col[i][ptr];
	    if (c == lastMovedCard)
		continue;
	    // Try moving this card to discard
	    int suit = c / 13;
	    int rank = c % 13;
	    if (((disc[suit] + 1) & 255) == rank) {
		game *g = new game(this, i * 10 + 41);
		int res = g->solve();
		if (res != PRUNED)
		    return res;
		else
		    delete g;
	    }
	    // Try moving this card to a free cell
	    for (int j = 0; j < 4; j++)
		if (fc[j] == EMPTY) {
		    game *g = new game(this, i* 10 + 40);
		    int res = g->solve();
		    if (res != PRUNED)
			return res;
		    else
			delete g;
		    break;
		}
	    // Try moving this card to a column
	    bool triedMovingToEmptyColumn = false;
	    for (int j = 0; j < 8; j++) {
		if (i == j)
		    continue;
		int ptr = colptr[j];
		if (ptr == -1) {
		    // Destination is empty column
		    if (triedMovingToEmptyColumn)
			continue;
		    else {
			game *g = new game(this, (i + 4) * 10 + j + 2);
			int res = g->solve();
			if (res != PRUNED)
			    return res;
			else {
			    delete g;
			    triedMovingToEmptyColumn = true;
			}
		    }
		} else {
		    // Destination is non-empty column
		    unsigned char topCard = col[j][colptr[j]];
		    int destSuit = topCard / 13;
		    int destRank = topCard % 13;
		    if (((suit ^ destSuit) & 1) == 0)
			continue;
		    if (rank != destRank + 1)
			continue;
		    game *g = new game(this, (i + 4) * 10 + j + 2);
		    int res = g->solve();
		    if (res != PRUNED)
			return res;
		    else
			delete g;
		}
	    }
	}

	// Couldn't find a card we could move
	gameHash[ourCrc] = newGameEntry->next;
	delete newGameEntry;
	return PRUNED;
    }
};

int main() {
    for (int i = 0; i < 65536; i++)
	gameHash[i] = NULL;
    deadline = timestamp();
    theGame = new game(stdin);
    theGame->solve();

    return 0;
}
