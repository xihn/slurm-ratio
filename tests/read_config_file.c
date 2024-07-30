/* TOML CONFIG PARSER */
#include <limits.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Declare the path to the configuration file */
const char *config_file = "config.toml";

/* Declare global vars of settings, and their default values 
 * to be updated when loading config or used on if disabled */
int enforce_ratio = 0;
int enforce_min = 0;
int enforce_max = 0;
char default_card[20] = "V100";
char partition[20] = "es1";

void parse_line(const char *line) {
    switch (firstvar) {

    }
}

void parse_settings(const char *line) {
    // Use strtok to split the line on the colon
    char key[50];
    char value[50];
    sscanf(line, "%[^:]:%s", key, value);

    // Compare the key and update the corresponding variable
    if (strcmp(key, "enforce_ratio") == 0) {
        enforce_ratio = (strcmp(value, "True") == 0) ? 1 : 0;
    } else if (strcmp(key, "default_card") == 0) {
        strncpy(default_card, value, sizeof(default_card) - 1);
        default_card[sizeof(default_card) - 1] = '\0'; // Ensure null-termination
    } else if (strcmp(key, "enforce_min") == 0) {
        enforce_min = (strcmp(value, "True") == 0) ? 1 : 0;
    } else if (strcmp(key, "enforce_max") == 0) {
        enforce_max = (strcmp(value, "True") == 0) ? 1 : 0;
    } else if (strcmp(key, "partition") == 0) {
        strncpy(partition, value, sizeof(partition) - 1);
        partition[sizeof(partition) - 1] = '\0'; // Ensure null-termination
    }

    // TODO: Check validity of each?
}

/* Main function for parsing config file*/
int read_config_file(const char *filename) {

    /* Check that config file exists*/
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr,"Error opening config file");
        return 0; // Since no config is found we just accept 
    }

    /* Loop for parsing */
    /*
    first parse for "enableGresRatio"
        if not found just return 0

    then look for "[gresratio]" and parse next 5 lines for settings
    
    then look for [gresratio.cards] and parse all cards as structs


    */
}

void print_config() {
    /* Should print: values of all settings
                     number of cards
                     settings of each card
    
    */
}

int main(int argc, char *argv[]) {
  if (argc < 1) {
      printf("Usage: %s <arg1>\n", argv[0]);
      return 1;
  }

  if (read_config_file(config_file)) {
    print_config();
    return 1;
  } else {
    fprintf(stderr,"Exited with error.");
    return 0;
  }
}

