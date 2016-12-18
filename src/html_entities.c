#include "html_entities.h"
#include "mem_ops.h"

char *html_entities(const char *str) 
{

  char *result = NULL;
  size_t n = 0, len = 0;
  const char *p=str;
  int i = 0;


  while (*p != '\0') 
  {
    const char *change;
    char buf[32];

    if (*p == '<')
      change = "&lt;";
    else if (*p == '>')
      change = "&gt;";
    else if (*p == '\\')
      change = "\\\\";
    else if (*p == '&')
      change = "&amp;";
    else if (*p == '"')
      change = "&quot;";
    else if (*p == '\'')
      change = "&apos;";
    else if (*p == '-' && p > str && *(p - 1) == '-') {
      change = "&#45;";
    } else if (*p < 0x20 || (unsigned char) *p > 0x7F) {
      snprintf(buf, sizeof(buf), "&#x%x;", (unsigned char) *p);
      change = buf;
    } else {

      buf[0] = *p;
      buf[1] = '\0';
      change = buf;
    }

    len = strlen(change);

    if ( !i || i + len > n) 
    {
      n = (i + len) << 1;
      result = xrealloc(result, n + 1);
    }

    memcpy(result + i, change, len);
    i += len;
    p++;
  }

  result = xrealloc(result, i + 1);
  result[i] = '\0';

//  if(result != NULL)
 ///  free(result);

  return result;
}

// sanitization without backslash
char *html_entities2(const char *str) 
{

  char *result = NULL;
  size_t n = 0, len = 0;
  const char *p=str;
  int i = 0;


  while (*p != '\0') 
  {
    const char *change;
    char buf[32];

    if (*p == '<')
      change = "&lt;";
    else if (*p == '>')
      change = "&gt;";
    else if (*p == '&')
      change = "&amp;";
    else if (*p == '"')
      change = "&quot;";
    else if (*p == '\'')
      change = "&apos;";
    else if (*p == '-' && p > str && *(p - 1) == '-') {
      change = "&#45;";
    } else if (*p < 0x20 || (unsigned char) *p > 0x7F) {
      snprintf(buf, sizeof(buf), "&#x%x;", (unsigned char) *p);
      change = buf;
    } else {

      buf[0] = *p;
      buf[1] = '\0';
      change = buf;
    }

    len = strlen(change);

    if ( !i || i + len > n) 
    {
      n = (i + len) << 1;
      result = xrealloc(result, n + 1);
    }

    memcpy(result + i, change, len);
    i += len;
    p++;
  }

  result = xrealloc(result, i + 1);
  result[i] = '\0';

//  if(result != NULL)
 ///  free(result);

  return result;
}



