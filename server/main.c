#include "stdio.h"

#include "utils/uuid.h"

int main() {
  char sessid[UTILS_SESSID_LEN];
  utils_generate_sessid(sessid);
  
  printf("%s\n", sessid);
}
