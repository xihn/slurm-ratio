work.c

#include <limits.h>
#include <regex.h>
#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <slurm/slurm_errno.h>`
// #include "src/slurmctld/slurmctld.h"

/* Required by Slurm job_submit plugin interface. */
const char plugin_name[] = "Require CPU/GPU ratio";
const char plugin_type[] = "job_submit/require_cpu_gpu_ratio";
// const uint32_t plugin_version = SLURM_VERSION_NUMBER;

/* Global variables. */
const char *myname = "job_submit_require_cpu_gpu_ratio";
const char *gpu_regex="^gpu:[_[:alnum:]:]*([[:digit:]]+)$"; // regex for gpu
const int  npart = 1; // number of partitions to check

/* The Partition to be checked - need to modify. */
const char *mypart[2] = {"es1"};
/* The CPU/GPU ratio that is checked against - need to modify. */
const int  ratio[2] = {2};

/* 
 Default values set below in case read from file is missing 


int enabled = 0;
int maxA40 = 16;
int maxV100 = 4;
int max2080TI = 2;
*/

/* Convert string to integer. */
int _str2int (char *str, uint32_t *p2int) {
    long int l;
    char *p;

    l = strtol(str, &p, 10);

    if ((*p != '\0') || (l > UINT32_MAX) || (l < 0)) {
        return -1;
    }

    *p2int = l;

    return 0;
}

/* Parses the config file*/
void parseConfigFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;

        // Check for enabled line
        if (strncmp(line, "enabled=", 8) == 0) {
            if (strcmp(line + 8, "True") == 0) {
                enabled = 1;
            } else {
                enabled = 0;
            }
        } else {
            // Parse JSON-like line
            char name[50];
            int min, max, weight;

            if (sscanf(line, "{\"name\":\"%[^\"]\", \"min\":%d, \"max\":%d, \"weight\":\"%d\"}", name, &min, &max, &weight) == 4) {
                if (strcmp(name, "2080ti") == 0) {
                    max2080TI = weight;
                } else if (strcmp(name, "v100") == 0) {
                    maxV100 = weight;
                } else if (strcmp(name, "a40") == 0) {
                    maxA40 = weight;
                }
            }
        }
    }

    fclose(file);
}

/* Main function*/
int _check_ratio(char *part, char *gres, uint32_t ncpu) {
    if (part == NULL) {
        printf("%s: missed partition info", myname);
        return 1;
    }

    /* Loop through all partitions that need to be checked. */
    int i;
    for (i = 0; i < npart; i++) {
        if (strcmp(part, mypart[i]) == 0) {
            /* Require GRES on a GRES partition. */
            if (gres == NULL) {
                printf("%s: missed GRES on partition %s", myname, mypart[i]);
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
                    
                    /* Escapes if enabled = 0*/
                    if (enabled == 0) {
                        return 1;
                    }

                    /* Sanity check of the CPU/GPU ratio. */
                    if (ncpu / ngpu <= 2) {
                        printf("%s: CPU=%zu, GPU=%zu, not qualify", myname, ncpu, ngpu);
                        return 0;
                    }

                } else if (rv == REG_NOMATCH) { /* no match */
                    printf("%s: missed GPU on partition %s", myname, mypart[i]);
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

extern int job_submit(struct job_descriptor *job_desc, uint32_t submit_uid,
        char **err_msg) {
    return _check_ratio(job_desc->partition,
                        job_desc->tres_per_node,
                        job_desc->min_cpus);
}

extern int job_modify(struct job_descriptor *job_desc,
        struct job_record *job_ptr, uint32_t submit_uid) {
    return _check_ratio(
        job_desc->partition == NULL ? job_ptr->partition : job_desc->partition,
        job_desc->tres_per_node == NULL ? job_ptr->tres_per_node : job_desc->tres_per_node,
        job_desc->min_cpus == (uint32_t) -2 ? job_ptr->total_cpus :
             job_desc->min_cpus);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <arg1> <arg2> <arg3>\n", argv[0]);
        return 1;
    }

    parseConfigFile("asdf.json");

    printf("Partition: %s\n", argv[1]);
    printf("gres: %s\n", argv[2]);
    printf("ncpu: %s\n", argv[3]);

    uint32_t temper = atoi(argv[3]);
    int v = _check_ratio(argv[1], argv[2], temper);

    printf("Result: %d\n", v);
    return 0;
}


