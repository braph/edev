#ifndef _XML2STD
#define _XML2STD

#include <libxml/parser.h>
#include <libxml/tree.h>

/* Converts */

static const char * const DEFAULT_PLAYINGINFO_FORMAT_TOP =
  "<text fg='black'>&lt;&lt; </text><title bold='on' fg='yellow' /><text fg='black'> &gt;&gt;</text>";

void() {
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;

  doc = xmlReadDoc(DEFAULT_PLAYINGINFO_FORMAT_TOP, NULL, 0);
  if (! doc)
    return;

  root_element = xmlDocGetRootElement(doc);

  xmlFreeDoc(doc);
  xmlCleanupParser();



}



#endif
