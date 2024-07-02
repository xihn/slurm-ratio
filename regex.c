#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256
#define MAX_ENTRIES 100

struct Card {
    char name[MAX_LINE_LENGTH];
    int min;
    int max;
    char weight[MAX_LINE_LENGTH];
};

struct Card entries[MAX_ENTRIES];
int num_entries = 0;

// Function to parse a line containing JSON-like object
void parse_line(char *line) {
    if (num_entries >= MAX_ENTRIES) {
        fprintf(stderr, "Max entries exceeded\n");
        return;
    }

    // Parse the line assuming fixed format
    if (sscanf(line, "{\"name\":\"%[^\"]\", \"min\":%d, \"max\":%d, \"weight\":\"%[^\"]\"}",
               entries[num_entries].name, &entries[num_entries].min,
               &entries[num_entries].max, entries[num_entries].weight) == 4) {
        num_entries++;
    } else {
        printf("Error parsing line: %s\n", line);
    }
}

// Function to search for an entry by name
struct Card* find_entry(const char *name) {
    for (int i = 0; i < num_entries; ++i) {
        if (strcmp(entries[i].name, name) == 0) {
            return &entries[i];
        }
    }
    return NULL;  // Card not found
}

int main() {
    FILE *file;
    char line[MAX_LINE_LENGTH];
    char search_name[MAX_LINE_LENGTH];

    // Open the file
    file = fopen("data.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Read each line and parse
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character if present
        line[strcspn(line, "\n")] = 0;
        parse_line(line);
    }

    // Close the file
    fclose(file);

    // Ask user for input name
    printf("Enter the name to search: ");
    if (fgets(search_name, sizeof(search_name), stdin)) {
        // Remove newline character if present
        search_name[strcspn(search_name, "\n")] = 0;

        // Find the entry by name
        struct Card *found_entry = find_entry(search_name);
        if (found_entry != NULL) {
            // Print the found entry details
            printf("Name: %s, Min: %d, Max: %d, Weight: %s\n",
                   found_entry->name, found_entry->min, found_entry->max,
                   found_entry->weight);
        } else {
            printf("Card with name '%s' not found.\n", search_name);
        }
    } else {
        printf("Error reading input.\n");
    }

    return 0;
}
