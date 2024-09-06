#include "libanim/html.h"

#include <stdlib.h>
#include <string.h>

#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>

int load_document(const char *contents, animDocument *document) {
    document->html = htmlReadMemory(contents, strlen(contents), NULL, NULL,
                                    HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING |
                                        HTML_PARSE_RECOVER);
    if (!document->html)
        return -1;

    document->context = xmlXPathNewContext(document->html);
    if (!document->context)
        return -1;

    return 0;
}

void unload_document(const animDocument *document) {
    if (document == NULL)
        return;
    xmlXPathFreeContext(document->context);
    xmlFreeDoc(document->html);
}

char *xpath_s(const char *expression, const animDocument *document) {
    xmlXPathObjectPtr obj = xpath(expression, document);

    char *result = (char *)xmlXPathCastToString(obj);

    xmlXPathFreeObject(obj);

    return result;
}

char *xpath_ns(const char *expression, const xmlNodePtr node,
               const animDocument *document) {
    xmlXPathObjectPtr obj = xpath_n(expression, node, document);

    char *result = (char *)xmlXPathCastToString(obj);

    xmlXPathFreeObject(obj);

    return result;
}

xmlXPathObjectPtr xpath(const char *expression, const animDocument *document) {
    return xmlXPathEvalExpression((const xmlChar *)expression,
                                  document->context);
}

xmlXPathObjectPtr xpath_n(const char *expression, const xmlNodePtr node,
                          const animDocument *document) {
    return xmlXPathNodeEval(node, (const xmlChar *)expression,
                            document->context);
}
