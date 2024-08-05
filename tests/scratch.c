#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define BUFFER_SIZE 2048
#define MAX_LINE_LENGTH 40
#define SWITCH "enable_gres_ratio_plugin"
#define SECTION "[gresratio]"
#define ENABLED_PATTERN "=\\s*true\\b"
#define DISABLED_PATTERN "=\\s*false\\b"

int disabled = 1; // defaults to Enabled or 1


/*  Config variables default values, updated when config is loaded */
int enforce_ratio = 0;
int enforce_min = 0;
int enforce_max = 0;
char default_card[MAX_LINE_LENGTH] = "V100";
char partition[MAX_LINE_LENGTH] = "es1";

/* Card data structure */
struct card {
  char name[MAX_LINE_LENGTH];
  int min;
  int max;
  char weight[MAX_LINE_LENGTH];
};

int parse_enabled(const char *line) {
    regex_t regex;
    int ret;
    ret = regcomp(&regex, ENABLED_PATTERN, REG_EXTENDED | REG_ICASE);

    if (ret) {
        fprintf(stderr, "Could not compile regex\n");
        return EXIT_FAILURE; // ESLURM_INTERNAL
    }

    ret = regexec(&regex, line, 0, NULL, 0);
    if (!ret) {
        printf("true\n"); // Match found
        return 0; // True
    } else if (ret == REG_NOMATCH) {
        printf("safd : False\n"); // No match
        return 1; // False
    } else {
        char msgbuf[100];
        regerror(ret, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        regfree(&regex);
        return EXIT_FAILURE; // ESLURM_INTERNAL
    }
    regfree(&regex);
}

int parse_boolean(const char *line) {
    regex_t regex;
    int ret;
    ret = regcomp(&regex, ENABLED_PATTERN, REG_EXTENDED | REG_ICASE);

    if (ret) {
        fprintf(stderr, "Could not compile regex\n");
        return EXIT_FAILURE; // ESLURM_INTERNAL
    }

    ret = regexec(&regex, line, 0, NULL, 0);
    if (!ret) {
        printf("true\n"); // Match found
        return 0; // True
    } else if (ret == REG_NOMATCH) {
        printf("asdf : False\n"); // No match
        return 1; // False
    } else {
        char msgbuf[100];
        regerror(ret, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        regfree(&regex);
        return EXIT_FAILURE; // ESLURM_INTERNAL
    }
    regfree(&regex);
}

char* parse_string(const char *line) {
    const char *pattern = "=\\s*([^\\s]+)";
    regex_t regex;
    regmatch_t match[2]; // Array to hold match positions

    // Compile the regex
    int reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return NULL;
    }

    // Execute the regex
    reti = regexec(&regex, line, 2, match, 0);
    if (!reti) {
        // Extract the matched substring
        int start = match[1].rm_so;
        int end = match[1].rm_eo;
        int length = end - start;

        // Allocate memory for the captured value
        char *value = malloc(length + 1);
        if (value == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            regfree(&regex);
            return NULL;
        }

        strncpy(value, &line[start], length);
        value[length] = '\0'; // Null-terminate the string

        // Free memory allocated to the pattern buffer by regcomp()
        regfree(&regex);

        return value; // Return the captured value
    } else if (reti == REG_NOMATCH) {
        regfree(&regex);
        return NULL; // No match found
    } else {
        char errbuf[100];
        regerror(reti, &regex, errbuf, sizeof(errbuf));
        fprintf(stderr, "Regex match failed: %s\n", errbuf);
        regfree(&regex);
        return NULL;
    }
}


void read_config(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE; 
    }

    char *buffer = malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return EXIT_FAILURE;
    }

    while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
        if (strncmp(buffer, SWITCH, strlen(SWITCH)) == 0) {
            /* If enableGresRatioPlugin is False then accept job*/
            if(parse_enabled(buffer) == 1) {
                printf("Accepting Job\n");
                int disabled = 0;
                break;
            }
        }

        /* This could be done more efficently */

        if (strncmp(buffer, "enforce_ratio", strlen("enforce_ratio"))) {
            enforce_ratio = parse_boolean(buffer);
        }

        if (strncmp(buffer, "default_card", strlen("default_card"))) {
            // default_card = parse_string(buffer);
            char *result = parse_string(buffer);

            if (result) {
                // Ensure the result fits in the global array
                strncpy(default_card, result, MAX_LINE_LENGTH - 1);
                default_card[MAX_LINE_LENGTH - 1] = '\0'; // Ensure null-termination

                printf("Captured value: %s\n", default_card);
                free(result); // Free the dynamically allocated memory
            } else {
                printf("No match found\n");
                default_card[0] = '\0'; // Clear the global array if no match is found
            }
        }

        if (strncmp(buffer, "enforce_min", strlen("enforce_min"))) {
            enforce_min = parse_boolean(buffer);
        }

        if (strncmp(buffer, "enforce_max", strlen("enforce_max"))) {
            enforce_max = parse_boolean(buffer);
        }

        if (strncmp(buffer, "partition", strlen("partition"))) {
            partition = parse_string(buffer);
        }
    }

    /* Free up space */
    free(buffer);
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    read_config(argv[1]);
    return EXIT_SUCCESS;
}