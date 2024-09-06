#ifndef LIBANIM_HTML_H
#define LIBANIM_HTML_H

#include "libxml/xpath.h"
#include <stddef.h>

typedef struct animDocument {
    void *html;    // Internal
    void *context; // Internal
} animDocument;

/**
 * Loads document from the content using libxml2
 *
 * @param contents Html body
 * @param document Variable to put parsed pointers in
 * @return 0 if success
 */
int load_document(const char *contents, animDocument *document);

/**
 * Frees created pointers for document
 *
 * @param document Document to be freed
 */
void unload_document(const animDocument *document);

/**
 * Evaluates a XPath expression and returns it as a string.
 *
 * @param expression XPath expression
 * @param document Document to evaluate on
 * @return The resulting string from evaluation
 */
char *xpath_s(const char *expression, const animDocument *document);

/**
 * Evaluates a XPath expression on a node and returns it as a string
 *
 * @param expression XPath expression
 * @param node Node to evaluate on
 * @param document Document to evaluate on
 * @return The resulting string from evaluation
 */
char *xpath_ns(const char *expression, const xmlNodePtr node,
               const animDocument *document);

/**
 * Evaluates a XPath expression and returns the libxml object.
 * Usable for multiple selections because libxml2 only supports XPath 1.0
 *
 * @param expression XPath expression
 * @param document Document to evaluate on
 * @return The resulting xml object from evaluation
 */
xmlXPathObjectPtr xpath(const char *expression, const animDocument *document);

/**
 * Evaluates a XPath expression on a node and returns the libxml object.
 * Usable for multiple selections because libxml2 only supports XPath 1.0
 *
 * @param expression XPath expression
 * @param node Node to evaluate on
 * @param document Document to evaluate on
 * @return The resulting xml object from evaluation
 */
xmlXPathObjectPtr xpath_n(const char *expression, const xmlNodePtr node,
                          const animDocument *document);

#endif
