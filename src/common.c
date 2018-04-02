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
    string_list = realloc(string_list, sizeof(char*) * (++count));

    if (string_list == NULL) {
      printf("Error! failed to allocate memory\n");
      return NULL;
    }

    string_list[count] = strtok (NULL, delimiter);
  }

  return string_list;
}