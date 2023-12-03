#include <stdio.h>
#include <stdlib.h>

#include "uuid4/uuid4.h"

int main() {
  char uuid[UUID4_LEN];

  uuid4_init();
  uuid4_generate(uuid);
  printf("%s\n", uuid);

  return EXIT_SUCCESS;
}
