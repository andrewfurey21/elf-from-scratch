#include <string.h>

typedef unsigned long long u64;

extern void print_string(const char * str, u64 len);

int main() {
  const char * str = "if this prints, it works!\n";
  u64 len = strlen(str);
  print_string(str, len);
}
