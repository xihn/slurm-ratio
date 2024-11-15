#include <limits.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 2048
#define MAX_LINE_LENGTH 40
#define MAX_ENTRIES 20
#define SWITCH "enable_gres_ratio_plugin"
#define SECTION "[gresratio]"
#define ENABLED_PATTERN "=[ \t]*(true)"
// #define DISABLED_PATTERN "=\\s*false\\b"
#define EQUALS_PATTERN "=[ \t]*([a-zA-Z0-9.]+)"
#define NAME_PATTERN "card\\.([a-zA-Z0-9]+)"

int disabled = 0; // defaults to false or 0 or disabled

/*  Config variables default values, updated when config is loaded */
char default_card[MAX_LINE_LENGTH] = "V100";
char partition[MAX_LINE_LENGTH] = "example";

/* Global variables. */
const char *myname = "job_submit_require_cpu_gpu_ratio";

/* Card data structure */
struct card {
  char name[MAX_LINE_LENGTH];
  float ratio;
};

/* Card information array */
struct card entries[MAX_ENTRIES];
int num_entries = 0;

/* Parses a line for a boolean value after an equals sign. ex: example = false -> 0 */
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
        //printf("true\n"); // Match found
        return 0; // False
    } else if (ret == REG_NOMATCH) {
        //printf("asdf : False\n"); // No match
        return 1; // True
    } else {
        char msgbuf[100];
        regerror(ret, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        regfree(&regex);
        return EXIT_FAILURE; // ESLURM_INTERNAL
    }
    regfree(&regex);
}

/* Parses a string after an equals sign. Ex partition = es1 -> es1*/
char* parse_string(const char *line, const char *pattern) {
    // const char *pattern = "=\\ *([^\\ ]+)";
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
        exit(0); 
    }

    char *buffer = malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(file);
        exit(0);
    }

    while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
        if (strncmp(buffer, SWITCH, strlen(SWITCH)) == 0) {
            /* If enableGresRatioPlugin is False then accept job*/
            if(parse_boolean(buffer) == 1) {
                //printf("Accepting Job\n");
                disabled = 1;
                break;
                }
            }

        if (strncmp(buffer, "default_card", strlen("default_card")) == 0) {
            // default_card = parse_string(buffer);
            char *result = parse_string(buffer, EQUALS_PATTERN);

            if (result) {
                // Ensure the result fits in the global array
                strncpy(default_card, result, MAX_LINE_LENGTH - 1);
                default_card[MAX_LINE_LENGTH] = '\0'; // Ensure null-termination

                //printf("Captured value: %s\n", default_card);
                free(result); // Free the dynamically allocated memory
            } else {
                printf("No match found for default card \n"); // error no default card provided in config
                // default_card[0] = '\0'; // Clear the global array if no match is found
                }
            }

        if (strncmp(buffer, "partition", strlen("partition")) == 0) {
            char *result = parse_string(buffer, EQUALS_PATTERN);

            if (result) {
                // Ensure the result fits in the global array
                strncpy(partition, result, MAX_LINE_LENGTH - 1);
                partition[MAX_LINE_LENGTH] = '\0'; // Ensure null-termination

                //printf("Captured value: %s\n", default_card);
                free(result); // Free the dynamically allocated memory
            } else {
                printf("No match found for partition\n");
                // default_card[0] = '\0'; // Clear the global array if no match is found
                }
            }

        if (strncmp(buffer, "card.", strlen("card.")) == 0) {
            char *result = parse_string(buffer, EQUALS_PATTERN);
            char *name = parse_string(buffer, NAME_PATTERN);

            if (result && name) {
                // Ensure the result fits in the global array
                float dub_ratio = strtof(result, NULL);
                entries[num_entries].ratio = dub_ratio; 
                strcpy(entries[num_entries].name, name);
                free(result);
                num_entries += 1;
            } else {
                printf("No match found for %s \n", buffer);

                }
            }
        }
    }

void print_config() {
    printf("disabled: %d\n", disabled);    
    printf("default_card: %s\n", default_card);
    printf("partition: %s\n", partition);

    for (int i = 0; i < num_entries; i++) {
        printf("Card Name: %s, Ratio: %f\n", entries[i].name, entries[i].ratio);
    }

    /* for card in entries: print card.name and card.ratio*/
}

/* Function to find the index of a card by name in entries */
int find_card_index(const char *card_name) {
    for (int i = 0; i < num_entries; i++) {
        if (strcasecmp(entries[i].name, card_name) == 0) {
            return i;
        }
    }
    return -1; // Card not found
}

/* Main function */
int _check_ratio(char *part, char *gres, uint32_t ncpu) {
    if (part == NULL) {
        printf("%s: missed partition info", myname);
        return 1;
    }

    const int npart = 1; 


    /* Loop through all partitions that need to be checked. */
    int i;
    for (i = 0; i < npart; i++) {
        if (strcmp(part, &partition[i * sizeof(partition)]) == 0) {
            /* Require GRES on a GRES partition. */
            if (gres == NULL) {
                printf("%s: missed GRES on partition %s", myname, &partition[i]);
                return 0;
            } else {
                char card_name[MAX_LINE_LENGTH];
                int gpu_count;
                /* Check char *gres to see if it has gpu:NUMBER or if it has gpu:NAME:NUMBER by counting ":" 
                        if it has gpu:NUMBER then set 
                
                */
                // Check if gres has a card name (format "gpu:name:x")
                if (sscanf(gres, "gpu:%39[^:]:%d", card_name, &gpu_count) == 2) {
                    // Format with card name found
                } else if (sscanf(gres, "gpu:%d", &gpu_count) == 1) {
                    // No card name, use default
                    strncpy(card_name, default_card, MAX_LINE_LENGTH);
                } else {
                    // Invalid format
                    return 0;
                }
                
                // Calculate the CPU-to-GPU ratio
                float ratio = (float)ncpu / gpu_count;
                
                // Find the card index in entries
                int index = find_card_index(card_name);
                if (index == -1) {
                    // Card not found in entries, handle error as needed
                    return 0;
                }
                
                // Compare ratios
                if (ratio == entries[index].ratio) {
                    printf("Calculated ratio %f is equal than stored ratio %f. Returning True.\n", ratio, entries[index].ratio);
                    return 1; // False, calculated ratio is greater
                } else {
                    printf("Calculated ratio %f is less than or more than to stored ratio %f. Returning False.\n", ratio, entries[index].ratio);
                    return 0; // True, calculated ratio is less than or equal
                }
            }   
  }
 }
}


int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <arg1> <arg2> <arg3>\n", argv[0]);
        return 1;
    }

    char *config  = "config.toml";
    read_config(config);
    print_config();

    uint32_t temper = atoi(argv[3]);
    int v = _check_ratio(argv[1], argv[2], temper);

    printf(". \n");
    printf("Result: %d\n", v);
    if (v == 0) {
        printf("Refused\n");
    } else if (v == 1) {
        printf("Accepted\n");
    }
    
    return 0;
}