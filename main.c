#include "dynamic.h"

int main() {
    FILE * file = fopen("./templates/test.html", "r");
    DynaElement * root = createDynamicDocument(file);
    fclose(file);

    putDynamicData(root, "title", "My page");
    putDynamicData(root, "content", "Welcome to my very nice website!");
    putDynamicData(root, "flavour", "very nice innit bruv");

    printf("%s", generateDynamicString(root));
}
