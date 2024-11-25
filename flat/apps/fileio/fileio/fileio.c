#include "usertraps.h"
#include "fileio.h"

#define NUM_BYTES 200

void main (int argc, char *argv[])
{
  int i;
  unsigned int fd;
  char golden_data[NUM_BYTES];
  char read_data[NUM_BYTES];

  Printf("fileio (%d): ================= FILE I/O SANITY TEST STARTS ================\n", getpid());

  for (i = 0; i < NUM_BYTES; i++) {
    golden_data[i] = i;
  }

  fd = file_open("fileio_test1.txt", "w");
  file_write(fd, golden_data, NUM_BYTES);
  file_close(fd);

  fd = file_open("fileio_test1.txt", "r");
  file_read(fd, read_data, NUM_BYTES);
  file_close(fd);

  for (i = 0; i < NUM_BYTES; i++) {
    if (golden_data[i] != read_data[i]) {
      Printf("fileio (%d): FAIL - data mismatch at byte %d\n", getpid(), i);
      Printf("fileio (%d): FAIL - golden: %d vs actual: %d\n", getpid(), golden_data[i], read_data[i]);
      exitsim();
    }
  }

  Printf("fileio (%d): ================= FILE I/O SANITY TEST PASSED! ================\n\n\n", getpid());
}
