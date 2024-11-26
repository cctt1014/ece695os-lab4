#include "usertraps.h"
#include "fileio.h"

#define NUM_BYTES 4000
#define START_OFS 300

void main (int argc, char *argv[])
{
  int i;
  unsigned int fd;
  char golden_data[NUM_BYTES];
  char read_data[NUM_BYTES];
  int res;

  for (i = 0; i < NUM_BYTES; i++) {
    golden_data[i] = i;
  }

  Printf("fileio (%d): ================= FILE I/O SANITY TEST STARTS ================\n", getpid());

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

  Printf("fileio (%d): ================= FILE I/O SEEK_EOF TEST STARTS ================\n", getpid());
  fd = file_open("fileio_test2.txt", "w");
  file_write(fd, golden_data, NUM_BYTES/2);
  file_close(fd);

  fd = file_open("fileio_test2.txt", "r");
  file_read(fd, read_data, NUM_BYTES/2); // assert EOF
  res = file_read(fd, &read_data[NUM_BYTES/2], NUM_BYTES/2); // supposed to be failed due to EOF
  if (res != FILE_FAIL) {
    Printf("fileio (%d): FAIL - EOF read failed expected here.\n", getpid());
    exitsim();
  } else {
    Printf("fileio (%d): INFO - EOF read failed as expected.\n", getpid());
  }
  file_close(fd);

  fd = file_open("fileio_test2.txt", "w");
  file_seek(fd, START_OFS+NUM_BYTES, FILE_SEEK_END); // move curr_pos (START_OFS+NUM_BYTES) away from EOF
  file_write(fd, &golden_data[NUM_BYTES/2], NUM_BYTES/2);
  file_close(fd);

  fd = file_open("fileio_test2.txt", "r");
  file_seek(fd, START_OFS+NUM_BYTES+(NUM_BYTES/2), FILE_SEEK_CUR); // move curr_pos to the correct position
  file_read(fd, &read_data[NUM_BYTES/2], NUM_BYTES/2); // read the second half of data
  file_close(fd);

  for (i = 0; i < NUM_BYTES; i++) {
    if (golden_data[i] != read_data[i]) {
      Printf("fileio (%d): FAIL - data mismatch at byte %d\n", getpid(), i);
      Printf("fileio (%d): FAIL - golden: %d vs actual: %d\n", getpid(), golden_data[i], read_data[i]);
      exitsim();
    }
  }

  Printf("fileio (%d): ================= FILE I/O SEEK_EOF TEST PASSED! ================\n\n\n", getpid());
}
