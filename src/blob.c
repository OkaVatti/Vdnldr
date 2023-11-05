#include <stdio.h>
#include <libxml/HTMLparser.h>

// open that html file
FILE* htmlFile = fopen("start.html", "r");
if (!htmlFile == NULL) {
    perror("Failed to open HTML file");
    return 1;
}

// init libxml2
htmlDocPtr doc;
doc = htmlReadFile("start.html", NULL, 0);
if (doc == NULL) {
    fprintf(stderr, "Failed to parse HTML file\n");
    return 1;
}

// parse for blob: url
xmlXPathContextPtr context = xmlXPathNewContext(doc);
if (context == NULL) {
    fprintf(stderr, "Failed to create XPath context\n");
    return 1;
}
xmlXPathObjectPtr result;
result = xmlXPathEvalExpression((const xmlChar*)"//video/@src", context); // parse for the src attribute in the video tag
if (result == NULL) {
    fprintf(stderr, "Failed to evaluate XPath expression\n");
    return 1;
}

if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    fprintf(stderr, "No matching elements could be found\n");
    return 1;
}

xmlNodePtr nodes = result->nodesetval;
for  (int i = 0; i < nodes->nodeNr; i++) {
    xmlChar* url = xmlNodeListGetString(doc, nodes->nodeTab[i]->xmlChildrenNode, 1);
    printf("Blob URL: %s\n", url);

    // Write the blob URL to a txt file
    FILE* blobFile = fopen("blob.txt", "w");
    if (!blobFile == NULL) {
        perror("Failed to open blob file");
        return 1;
    }
    fprintf(blobFile, "%s", url);
    fclose(blobFile);
}

xmlXPathFreeObject(result);
xmlXPathFreeContext(context);
xmlFreeDoc(doc);
fclose(htmlFile);