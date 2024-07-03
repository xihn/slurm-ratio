/* (Old header from original plugin)
 * Copyright (c) 2016-2017, Yong Qin <yong.qin@lbl.gov>. All rights reserved.
 *
 * job_submit_require_cpu_gpu_ratio: Job Submit plugin to enfornce the
 *      CPU/GPU ratio to facilitate accounting.
 *
 * This plugin enforces the job to provide enough CPU number to match the GPU
 * number (e.g, 2:1) on a particular partition. If the provided number does not
 * match the job will be refused.
 *
 * Note you will need to change the definition of "mypart" and "ratio" in the
 * following code to meet your own requirement.
 *
 * gcc -shared -fPIC -pthread -I${SLURM_SRC_DIR}
 *     job_submit_require_cpu_gpu_ratio.c -o job_submit_require_cpu_gpu_ratio.so
 *
 */

#include <limits.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256
#define MAX_ENTRIES 50
#define MAX_MATCHES 10
#define MAX_GROUPS 2
#define MAX_LINE_LENGTH 256

// #include <slurm/slurm_errno.h>`
// #include "src/slurmctld/slurmctld.h"

/* Required by Slurm job_submit plugin interface. */
const char plugin_name[] = "Require CPU/GPU ratio";
const char plugin_type[] = "job_submit/require_cpu_gpu_ratio";
// const uint32_t plugin_version = SLURM_VERSION_NUMBER;

/* Global variables. */
const char *myname = "job_submit_require_cpu_gpu_ratio";      // slurm requires?
const char *gpu_regex = "^gpu:[_[:alnum:]:]*([[:digit:]]+)$"; // regex for gpu

const int npart = 1; // number of partitions to check
const int ratio = 2; // ratio required

/* Vars needed for config file */
const char *config_file = "job_submit_ratio_config.yml"; // name of config file
FILE *file;
char line[MAX_LINE_LENGTH];
char search_name[MAX_LINE_LENGTH];

/*  Config variables default values, updated when config is loaded*/
int enforce_ratio = 0;
int enforce_min = 0;
int enforce_max = 0;
char default_card[20] = "V100";
char partition[20] = "es1";

struct card {
  char name[MAX_LINE_LENGTH];
  int min;
  int max;
  char weight[MAX_LINE_LENGTH];
};

struct card entries[MAX_ENTRIES];
int num_entries = 0;

/* Convert string to integer. */
int _str2int(char *str, uint32_t *p2int) {
  long int l;
  char *p;

  l = strtol(str, &p, 10);

  if ((*p != '\0') || (l > UINT32_MAX) || (l < 0)) {
    return -1;
  }

  *p2int = l;

  return 0;
}

// Function to parse a line containing JSON-like object
void _parse_line(char *line) {
  if (num_entries >= MAX_ENTRIES) {
    fprintf(stderr, "Max entries exceeded\n");
    return;
  }

  // Parse the line assuming fixed format
  if (sscanf(line,
             "{\"name\":\"%[^\"]\", \"min\":%d, \"max\":%d, "
             "\"weight\":\"%[^\"]\"}",
             entries[num_entries].name, &entries[num_entries].min,
             &entries[num_entries].max, entries[num_entries].weight) == 4) {
    num_entries++;
  } else {
    printf("Error parsing line: %s\n", line);
  }
}

// Function to search for an entry by name
struct card *find_entry(const char *name) {
  for (int i = 0; i < num_entries; ++i) {
    if (strcmp(entries[i].name, name) == 0) {
      return &entries[i];
    }
  }
  return NULL; // Card not found
}

void _update_config(const char *line) {
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
}

/* Parses the config file*/
void read_config_file(const char *filename) {

  // Open the file
  file = fopen(filename, "r");
  if (file == NULL) {
    perror("Error opening file");
    return;
  }

  // Skip and parse the first 5 lines, update config global vars
  for (int i = 0; i < 5; ++i) {
    if (fgets(line, sizeof(line), file)) {
      _update_config(line);
    } else {
      fprintf(stderr, "Error reading file or file has less than 5 lines.\n");
      fclose(file);
      return;
    }
  }

  // Read each line and parse
  while (fgets(line, sizeof(line), file)) {
    // Remove newline character if present
    line[strcspn(line, "\n")] = 0;
    _parse_line(line);
  }

  // Close the file
  fclose(file);
}

/* Gets the card name from gres */
int capture_card(const char *input, char *alphanumeric) {
  regex_t regex;
  regmatch_t matches[MAX_MATCHES];
  char pattern[] = "^gpu:([[:alnum:]]+):([[:digit:]]+)$";
  int ret;

  // Compile the regex pattern
  ret = regcomp(&regex, pattern, REG_EXTENDED);
  if (ret != 0) {
    fprintf(stderr, "Failed to compile regex pattern\n");
    return -1;
  }

  // Execute the regex search
  ret = regexec(&regex, input, MAX_MATCHES, matches, 0);
  if (ret == 0 && matches[1].rm_so != -1) {
    // Match found, extract the alphanumeric part
    int length = matches[1].rm_eo - matches[1].rm_so;
    strncpy(alphanumeric, input + matches[1].rm_so, length);
    alphanumeric[length] = '\0'; // Null-terminate the string
    regfree(&regex);             // Free allocated memory for regex
    return 0;
  } else if (ret == REG_NOMATCH) {
    regfree(&regex); // Free allocated memory for regex
    return -1;       // No match found
  } else {
    char errbuf[100];
    regerror(ret, &regex, errbuf, sizeof(errbuf));
    fprintf(stderr, "Regex match failed: %s\n", errbuf);
    regfree(&regex); // Free allocated memory for regex
    return -1;
  }
}

/* Main function */
int _check_ratio(char *part, char *gres, uint32_t ncpu) {
  if (part == NULL) {
    printf("%s: missed partition info", myname);
    return 1;
  }

  /* Loop through all partitions that need to be checked. */
  int i;
  for (i = 0; i < npart; i++) {
    if (strcmp(part, &partition[i * sizeof(partition)]) == 0) {
      /* Require GRES on a GRES partition. */
      if (gres == NULL) {
        printf("%s: missed GRES on partition %s", myname, &partition[i]);
        return 0;
      } else {
        regex_t re;
        regmatch_t rm[2];

        /* Number of GPUs. */
        uint32_t ngpu = 0;

        if (regcomp(&re, gpu_regex, REG_EXTENDED) != 0) {
          printf("%s: failed to compile regex '%s': %m", myname, gpu_regex);
          return 0;
        }

        int rv = regexec(&re, gres, 2, rm, 0);

        regfree(&re);

        if (rv == 0) { /* match */
            /* Convert the GPU # to integer. */
            if (_str2int(gres + rm[1].rm_so, &ngpu) || ngpu < 1) {
                printf("%s: invalid GPU number %s", myname, gres + rm[1].rm_so);
                return 0;
            }

            /* Vars */
            char card_name[MAX_LINE_LENGTH];

            // sets card_name to whatever it finds from gres or default
            if (capture_card(gres, card_name) == 0) {
                printf("matched card name: %s\n", card_name);
            } else { // set default card
                strncpy(card_name, default_card, sizeof(card_name) - 1);
                card_name[sizeof(card_name) - 1] = '\0'; // Ensure null-termination
                printf("Using Default card since no card was specified.\n");
            }

            struct card *found_entry = find_entry(card_name);
            if (found_entry != NULL) {
            // Print the found card details
            printf("Name: %s, Min: %d, Max: %d, Weight: %s\n", found_entry->name, found_entry->min, found_entry->max, found_entry->weight);

                /* Enforce minimum cpu requirements*/
                if (enforce_min) {
                    if (ncpu < found_entry->min * ngpu) {
                        printf("Cpu for %s lower than min %d per card", found_entry->name, found_entry->min);
                        return 0;
                    }
                }

                /* Enforce maximum cpu requirements*/
                if (enforce_max) {
                    if (ncpu > found_entry->max * ngpu) {
                        printf("Cpu for %s higher than max %d per card", found_entry->name, found_entry->max);
                        return 0;
                    }
                }

                /* Enforce ratio*/
                if (enforce_ratio) {
                    int weight_int = atoi(found_entry->weight);
                    if (((double) ncpu / (ngpu * weight_int)) <= (double) ratio) {
                        // info("%s: CPU=%zu, GPU=%zu, not qualify", myname, ncpu,
                        // ngpu);
                        printf("Ratio of %.2f is lower than %d", (double) ncpu / (ngpu * weight_int), ratio);
                        return 0;
                    }
                }

          } else if (found_entry == NULL) {
            printf("Card with name '%s' not found in config.\n", search_name);
          }

        } else if (rv == REG_NOMATCH) { /* no match */
          printf("%s: missed GPU on partition %s", myname, &partition[i]);
          return 0;
        } else { /* error */
          printf("%s: failed to match regex '%s': %m", myname, gpu_regex);
          return 0;
        }
      }
    }
  }
  printf("Sucess");
  return 1;
}

int main(int argc, char *argv[]) {

  read_config_file(config_file);

  printf("partition: %s\n", argv[1]);
  printf("gres: %s\n", argv[2]);
  printf("ncpu: %s\n", argv[3]);

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