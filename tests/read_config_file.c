#include <limits.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256
#define MAX_ENTRIES 50

/* Declare the path to the configuration file */
const char *config_file = "config.toml";

/* Declare global vars of settings, and their default values 
 * to be updated when loading config or used on if disabled */
int enforce_ratio = 0;
int enforce_min = 0;
int enforce_max = 0;
char default_card[20] = "V100";
char partition[20] = "es1";

/* Data Structure for storing card information */
struct card {
    char name[MAX_LINE_LENGTH];
    int min;
    int max;
    float weight;
};

struct card cards[MAX_ENTRIES];
int num_cards = 0;

void parse_settings(const char *line) {
    char key[50];
    char value[50];
    sscanf(line, "%[^:]:%s", key, value);

    if (strcmp(key, "enforce_ratio") == 0) {
        enforce_ratio = (strcmp(value, "true") == 0) ? 1 : 0;
    } else if (strcmp(key, "default_card") == 0) {
        strncpy(default_card, value, sizeof(default_card) - 1);
        default_card[sizeof(default_card) - 1] = '\0';
    } else if (strcmp(key, "enforce_min") == 0) {
        enforce_min = (strcmp(value, "true") == 0) ? 1 : 0;
    } else if (strcmp(key, "enforce_max") == 0) {
        enforce_max = (strcmp(value, "true") == 0) ? 1 : 0;
    } else if (strcmp(key, "partition") == 0) {
        strncpy(partition, value, sizeof(partition) - 1);
        partition[sizeof(partition) - 1] = '\0';
    }
}

void parse_line(const char *line, int in_gresratio_section) {
    if (in_gresratio_section) {
        parse_settings(line);
    }
}

void parse_card(const char *line) {
    char card_name[MAX_LINE_LENGTH];
    int min, max;
    float weight;

    if (sscanf(line, "%[^=] = {min = %d, max = %d, weight = %f}", card_name, &min, &max, &weight) == 4) {
        card_name[strcspn(card_name, " ")] = '\0';
        strncpy(cards[num_cards].name, card_name, MAX_LINE_LENGTH - 1);
        cards[num_cards].min = min;
        cards[num_cards].max = max;
        cards[num_cards].weight = weight;
        num_cards++;
    }
}

int read_config_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening config file\n");
        return 0;
    }

    char line[MAX_LINE_LENGTH];
    int in_gresratio_section = 0;
    int in_gresratio_cards_section = 0;
    int gresratio_found = 0;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';

        if (strcmp(line, "[gresratio]") == 0) {
            in_gresratio_section = 1;
            gresratio_found = 1;
            continue;
        }

        if (strcmp(line, "[gresratio.cards]") == 0) {
            in_gresratio_cards_section = 1;
            in_gresratio_section = 0;
            continue;
        }

        if (line[0] == '[') {
            in_gresratio_section = 0;
            in_gresratio_cards_section = 0;
            continue;
        }

        if (in_gresratio_section) {
            parse_line(line, in_gresratio_section);
        } else if (in_gresratio_cards_section) {
            parse_card(line);
        }
    }

    fclose(file);

    if (!gresratio_found) {
        fprintf(stderr, "Error: '[gresratio]' not found in config file\n");
        return 0;
    }

    return 1;
}

void print_config() {
    printf("enforce_ratio: %d\n", enforce_ratio);
    printf("default_card: %s\n", default_card);
    printf("enforce_min: %d\n", enforce_min);
    printf("enforce_max: %d\n", enforce_max);
    printf("partition: %s\n", partition);
    printf("Number of cards: %d\n", num_cards);

    for (int i = 0; i < num_cards; i++) {
        printf("Card %d:\n", i + 1);
        printf("  Name: %s\n", cards[i].name);
        printf("  Min: %d\n", cards[i].min);
        printf("  Max: %d\n", cards[i].max);
        printf("  Weight: %.1f\n", cards[i].weight);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <config_file>\n", argv[0]);
        return 1;
    }

    config_file = argv[1];

    if (read_config_file(config_file)) {
        print_config();
        return 0;
    } else {
        fprintf(stderr, "Exited with error.\n");
        return 1;
    }
}
