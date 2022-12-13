#include "dynamic.h"
#include <string.h>
#include <stdlib.h>

void promptUserData(DynaElement *element, char *key, char *prompt, int max_size);
void createDirectory(char *name);
void clearStreamBuffer();

int main() {
    // Dynamic HTML Element (for root)
    FILE *file = fopen("./templates/index.html", "r");
    DynaElement *root = createDynamicDocument(file);
    fclose(file);

    // Data request
    promptUserData(root, "company", "Company Name", 20);
    promptUserData(root, "accent", "Page Accent colour (please use the following HEX format: RRGGBB - do NOT add a #)", 20);
    promptUserData(root, "description", "Company Description", 200);
    promptUserData(root, "splash", "Link to the image of the Splash shown in the background (.png)", 150);

    promptUserData(root, "logo", "Link to the image of the Logo, where the Icon and Text are visible (.svg, .png)", 150);
    promptUserData(root, "text_logo", "Link to the image of the Logo, where ONLY Text is visible (.svg, .png)", 150);
    promptUserData(root, "small_logo", "Link to the image of the Logo, where ONLY the Icon is visible (.svg, .png)", 150);

    // Team size
    printf("\nEnter the amount of team members (positive integer):\n> ");
    int team_size = 0;
    while (team_size <= 0) {
        if (scanf("%d", &team_size) != 1) {
            // mark as error
            team_size = 0;
            // clear buffer
            clearStreamBuffer();
        }
        if (team_size <= 0) {
            printf("! Invalid Input ! Must be a positive integer\n");
            printf("\nEnter the amount of team members (positive integer):\n> ");
        }
    }
    clearStreamBuffer();

    // Dynamic HTML Element (for a single member)
    file = fopen("./templates/team_member.html", "r");

    // The Team HTML Builder
    char *team_html = calloc(1, 1);
    // The current size of the Team HTML
    uint64_t team_html_size = 0;

    // Each team member
    for (int i = 1; i <= team_size; i++) {
        printf("\n\n---\nTeam member No. %d\n", i);

        // Create the Team Member element
        DynaElement *member = createDynamicDocument(file);

        // Fill the data
        promptUserData(member, "name", "Name of the Team Member", 80);
        promptUserData(member, "role", "The Role of the Team Member", 50);
        promptUserData(member, "avatar", "Link to the image of the member, such as an avatar or portrait (.svg, .png)", 150);
        promptUserData(member, "colour", "Member Accent colour (please use the following HEX format: RRGGBB - do NOT add a #)", 150);

        // Save new Team HTML Size
        team_html_size += strlen(generateDynamicString(member)) + 1;
        // Expand the Team HTML String
        team_html = realloc(team_html, team_html_size);
        // Concatenate the new Member to the Team HTML
        strncat(team_html, member->result, team_html_size);

        // Free the Member
        freeDynamicElement(member);
    }

    // Close the single member file
    fclose(file);

    // Finally add the Team HTML as data and free the builder
    putDynamicData(root, "team", team_html);
    free(team_html);

    // Create the "./out" directory if missing
    createDirectory("./out");

    // Save the generated HTML
    FILE *result = fopen("./out/index.html", "w");
    fputs(generateDynamicString(root), result);
    fclose(result);

    // Free up memory
    freeDynamicElement(root);

    // Ready CSS copy
    FILE *source = fopen("./style.css", "r");
    FILE *target = fopen("./out/style.css", "w");

    // Fail if CSS was not detected
    if (source == NULL) {
        printf("Missing CSS file! Cannot finish!");
        return 1;
    }

    // Copy
    int ch;
    while((ch = fgetc(source)) != EOF)
        fputc(ch, target);

    // Close both
    fclose(source);
    fclose(target);

    // Final message
    printf("\n\n---\nYour website has been generated!\n");
    printf("Please check the \"./out/\" folder");
}

void promptUserData(DynaElement *element, char *key, char *prompt, int max_size) {
    printf("\nEnter %s:\n> ", prompt);

    char *value = calloc(max_size + 1, 1);
    fgets(value, max_size, stdin);
    // trailing '\n' trimming
    value[strcspn(value, "\n")] = 0;

    putDynamicData(element, key, "%s", value);
    free(value);
}

#ifdef __linux__
void createDirectory(char *name) {
    mkdir(name, 0775);
}
#elif defined(_WIN32)
#include <direct.h>
#include <stdio.h>

void createDirectory(char *name) {
    _mkdir(name);
}
#endif

void clearStreamBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}
