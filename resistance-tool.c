#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    const char* name;
    int digit;
    double multiplier;
    const char* tolerance;
    const char* ansi;
} Color;

static const Color DICTIONARY[] = {
    {"BLA",   0, 1e0,  NULL,    "\033[38;5;0m"},
    {"BRO",   1, 1e1,  "1%",    "\033[38;5;94m"},
    {"RED",   2, 1e2,  "2%",    "\033[91m"},
    {"ORA",   3, 1e3,  NULL,    "\033[38;5;208m"},
    {"YEL",   4, 1e4,  NULL,    "\033[93m"},
    {"GRE",   5, 1e5,  "0.5%",  "\033[32m"},
    {"BLU",   6, 1e6,  "0.25%", "\033[34m"},
    {"VIO",   7, 1e7,  "0.1%",  "\033[35m"},
    {"GRA",   8, 1e8,  "0.05%", "\033[38;5;239m"},
    {"WHI",   9, 1e9,  NULL,    "\033[97m"},
    {"GOL",  -1, 0.1,  "5%",    "\033[38;5;222m"},
    {"SIL",  -1, 0.01, "10%",   "\033[38;5;251m"}
};

static const char* reset = "\033[0m";

static void usage() {

    printf("\nUsage: resistance-tool is a utility to identify resistors.\n\n");
    printf("Encoding mode converts numerical resistance and tolerance values into 5-band or 4-band identification.\n");
    printf("Like this:    ./resistance-tool encode [band count] [resistance] [tolerance]\n\n");
    printf("Decoding mode converts 5-band or 4-band identifications back to numerical resistance and tolerance values.\n");
    printf("Like this:    ./resistance-tool decode [color 1] ... [color n]\n\n");

    exit(0);
}

static void error_handler(char* msg) {

    fprintf(stderr, "%s", msg);
    exit(1);

}

static int color_locator(const char* target) {
    for (int i = 0; i < 12; i++) {
        if (strcasecmp(target, DICTIONARY[i].name) == 0)
            return i;
    }
    return -1;
}

static int tolerance_locator(const char* target) {
    for (int i = 0; i < 12; i++) {
        if (DICTIONARY[i].tolerance != NULL && strcasecmp(target, DICTIONARY[i].tolerance) == 0)
            return i;
    }
    return -1;
}

static double convert(const char* reading) {
    
    double result = atof(reading);

    int len = strlen(reading);
    if (reading[len - 1] == 'k' || reading[len - 1] == 'K')
        return result *= 1e3;
    if (reading[len - 1] == 'M')
        return result *= 1e6;

    return result;
}

static void decode(int degree, char** query) {

    if (degree != 4 && degree != 5)
        error_handler("Unsupported encoding format.\n");

    int bank[5];
    for (int i = 0; i < degree; i++) {
        bank[i] = color_locator(query[i]);
        if (bank[i] == -1)
            error_handler("Invalid color code.\n");
    }

    double resistance;
    if (degree == 4) {
        resistance = DICTIONARY[bank[0]].digit * 10
                   + DICTIONARY[bank[1]].digit;
    }
    else {
        resistance = DICTIONARY[bank[0]].digit * 100
                   + DICTIONARY[bank[1]].digit * 10
                   + DICTIONARY[bank[2]].digit;
    }

    resistance *= DICTIONARY[bank[degree - 2]].multiplier;
    printf("Resistance: ");
    if (resistance >= 1e6) {
        printf("%.2f M    ", resistance / 1e6);
    } else if (resistance >= 1e3) {
        printf("%.2f k    ", resistance / 1e3);
    } else {
        printf("%.2f"    , resistance);
    }
    
    const char *tolerance = DICTIONARY[bank[degree - 1]].tolerance;
    if (tolerance == NULL) tolerance = "20%";
    printf("Tolerance: %s\n", tolerance);
}

static void encode(int degree, double resistance, char* tolerance) {
    
    if (degree != 4 && degree != 5)
        error_handler("Unsupported encoding format.\n");
    
    int exponent = (int)floor(log10(resistance));
    int sig_digits = (degree == 4) ? 2 : 3;
    exponent = exponent - (sig_digits - 1);
    
    double normalised = resistance / pow(10, exponent);
    int display = (int)round(normalised);

    if ((degree == 4 && display >= 100) || (degree == 5 && display >= 1000)) {
        display /= 10;
        exponent++;
    }

    int digits[3];
    if (degree == 4) {
        digits[0] = display / 10;
        digits[1] = display % 10;
    }
    else {
        digits[0] = display / 100;
        digits[1] = (display / 10) % 10;
        digits[2] = display % 10;
    }

    int mult_color = -1;
    for (int i = 0; i < 12; i++) {
        if (fabs(DICTIONARY[i].multiplier - pow(10, exponent)) < 1e-6) {
            mult_color = i;
            break;
        }
    }
    if (mult_color == -1)
        error_handler("Value out of representable range.\n");

    int tol_color = tolerance_locator(tolerance);
    
    printf("Degree %d representation: ", degree);
    for(int i = 0; i < sig_digits; ++i)
        printf("%s%s%s ", DICTIONARY[digits[i]].ansi, DICTIONARY[digits[i]].name, reset);
    
    printf("| %s%s%s ", DICTIONARY[mult_color].ansi, DICTIONARY[mult_color].name, reset);
    
    if (tol_color != -1)
        printf("| %s%s%s\n", DICTIONARY[tol_color].ansi, DICTIONARY[tol_color].name, reset);
    else
        printf("| Unrepresentable tolerance\n");
}


int main(int argc, char *argv[]) {
    
    if (argc < 5)
        usage();

    if (strcasecmp(argv[1], "encode") == 0) {
        if (argc != 5)
            usage();
        encode(atoi(argv[2]), convert(argv[3]), argv[4]);
        exit(0);
    }

    if (strcasecmp(argv[1], "decode") == 0) {
        if (argc != 5 && argc != 6)
            usage();
        decode(argc - 2, &argv[2]);
    }

    else
        usage();

}
