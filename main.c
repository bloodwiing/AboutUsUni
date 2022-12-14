#include "dynamic.h"
#include <string.h>
#include <stdlib.h>

#define MAX_LINK_SIZE 400
#define MAX_RGB_SIZE 8

void promptUserData(DynaElement *element, char *key, char *prompt, int max_size);
void createDirectory(char *name);
void clearStreamBuffer();

int main() {
    // Dynamic HTML Element (for root)
    FILE *file = fopen("./templates/index.html", "r");
    DynaElement *document = createDynamicDocument(file);
    fclose(file);

    // Light / Dark mode
    printf("\nDo you want the page to be in [L] light or [D] dark theme?\n(Please enter only one symbol matching the theme name)\n> ");
    char theme = 0;
    while (!theme) {
        if (scanf("%c", &theme) != 1) {
            // mark as error
            theme = 0;
            // clear buffer
            clearStreamBuffer();
        }
        if (!theme || (theme != 'l' && theme != 'L' && theme != 'd' && theme != 'D')) {
            // mark as error
            theme = 0;
            printf("! Invalid Input ! Must be L or D\n");
            printf("\nDo you want the page to be in [L] light or [D] dark theme?\n(Please enter only one symbol matching the theme name)\n> ");
        }
    }
    clearStreamBuffer();

    if (theme == 'l' || theme == 'L')
        putDynamicData(document, "theme", "data-light-mode");

    // Data request
    promptUserData(document, "company", "Company Name", 20);
    promptUserData(document, "accent", "Page Accent colour (please use the following HEX format: RRGGBB - do NOT add a #)", MAX_RGB_SIZE);
    promptUserData(document, "description", "Company Description", 200);
    promptUserData(document, "splash", "Link to the image of the Splash shown in the background (.png)", MAX_LINK_SIZE);

    promptUserData(document, "logo", "Link to the image of the Logo, where the Icon and Text are visible (.svg, .png)", MAX_LINK_SIZE);
    promptUserData(document, "text_logo", "Link to the image of the Logo, where ONLY Text is visible (.svg, .png)", MAX_LINK_SIZE);
    promptUserData(document, "small_logo", "Link to the image of the Logo, where ONLY the Icon is visible (.svg, .png)", MAX_LINK_SIZE);

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
        promptUserData(member, "avatar", "Link to the image of the member, such as an avatar or portrait (.svg, .png)", MAX_LINK_SIZE);
        promptUserData(member, "colour", "Member Accent colour (please use the following HEX format: RRGGBB - do NOT add a #)", MAX_RGB_SIZE);

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
    putDynamicData(document, "team", team_html);
    free(team_html);

    // Create the "./out" directory if missing
    createDirectory("./out");

    // Save the generated HTML
    FILE *result = fopen("./out/index.html", "w");
    fputs(generateDynamicString(document), result);
    fclose(result);

    // Free up memory
    freeDynamicElement(document);

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
    fgets(value, max_size + 1, stdin);

    // remove any overflowing symbols
    if (value[max_size - 1])
        clearStreamBuffer();

    // trailing '\n' trimming
    value[strcspn(value, "\n")] = 0;

    // null-terminate string (in case it is not)
    value[max_size - 1] = 0;

    putDynamicData(element, key, "%s", value);

    free(value);
}

#ifdef __linux__
#include <sys/stat.h>

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
