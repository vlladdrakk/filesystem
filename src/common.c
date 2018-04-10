#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
  This function will split the string and return a string array
  terminated by a null pointer. 
*/
char** strsplit(char* str, const char* delimiter) {
  char* string = strdup(str);

  size_t count = 0;
  char** string_list = malloc(sizeof(char*));

  string_list[count] = strtok(string, delimiter);
  while (string_list[count] != NULL)
  {
    count++;
    string_list = realloc(string_list, sizeof(char*) * count);

    if (string_list == NULL) {
      printf("Error! failed to allocate memory\n");
      return NULL;
    }

    string_list[count] = strtok (NULL, delimiter);
  }

  return string_list;
}



int str_split (const char *str, char *parts[], const char *delimiter) {
  char *pch;
  int i = 0;
  char *copy = NULL, *tmp = NULL;

  copy = strdup(str);
  if (! copy)
    goto bad;

  pch = strtok(copy, delimiter);

  tmp = strdup(pch);
  if (! tmp)
    goto bad;

  parts[i++] = tmp;

  while (pch) {
    pch = strtok(NULL, delimiter);
    if (NULL == pch) break;

    tmp = strdup(pch);
    if (! tmp)
      goto bad;

    parts[i++] = tmp;
  }

  free(copy);
  return i;

 bad:
  free(copy);
  int j;
  for (j = 0; j < i; j++)
    free(parts[j]);
  return -1;
}
