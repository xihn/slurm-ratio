#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define BUFFER_SIZE 1024
#define SWITCH "enableGresRatioPlugin"
#define SECTION "[gresratio]"

#define ENABLED_PATTERN "=\\s*true\\b"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "r");
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
            regex_t regex;
            int ret;
            ret = regcomp(&regex, ENABLED_PATTERN, REG_EXTENDED | REG_ICASE);

            if (ret) {
                fprintf(stderr, "Could not compile regex\n");
                return EXIT_FAILURE;
            }

            ret = regexec(&regex, buffer, 0, NULL, 0);
            if (!ret) {
                printf("true\n"); // Match found
                
            } else if (ret == REG_NOMATCH) {
                printf("gresratio not enabled, accepting job\n"); // No match
                // return SLURM_SUCESS
            } else {
                char msgbuf[100];
                regerror(ret, &regex, msgbuf, sizeof(msgbuf));
                fprintf(stderr, "Regex match failed: %s\n", msgbuf);
                regfree(&regex);
                return EXIT_FAILURE;
            }
            regfree(&regex);
            printf("test\n");
        }
    }
    free(buffer);
    fclose(file);
    return EXIT_SUCCESS;
}
