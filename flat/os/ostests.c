#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"

#define NUMBYTES 1024
#define NON_ALGIN_BYTE_AMOUNT 1030

#define NUM_TESTCASE 300

void RunOSTests() {
  // STUDENT: run any os-level tests here
  uint32 inode;
  int a, i;
  char big[NUMBYTES];
  char big2[NUMBYTES];
  char non_align_golden_data[NON_ALGIN_BYTE_AMOUNT];
  char non_align_data[NON_ALGIN_BYTE_AMOUNT];

  // printf("runostests (%d): ================= SANITY TEST STARTS ================\n", GetCurrentPid());
  // inode = DfsInodeOpen("ece595-file-1");

  // printf("runostests: inode after open is %d\n", inode);

  // for(a=0; a<1024; a++) {
  //   big[a] = a;
  // }

  // for(a=0; a<12; a++) {
  //   DfsInodeWriteBytes(inode, big, a*NUMBYTES, NUMBYTES);
  // }

  // DfsInodeReadBytes(inode, big2, 11*NUMBYTES, NUMBYTES);

  // for(a=0; a<NUMBYTES; a++) {
  //   if (big[a] != big2[a]) {
  //     printf("runostests (%d): FAIL: index big[%d] != big2[%d] (%d != %d)\n", GetCurrentPid(), a, a, big[a], big2[a]);
  //     GracefulExit();
  //   }
  // }

  // printf("runostests: ece595-file-1 ops worked!\n");

  // DfsInodeDelete(inode);

  // printf("runostests (%d): ================= SANITY TEST PASSED! ================\n", GetCurrentPid());


  printf("runostests (%d): ================= NON-BLOCK-ALIGN TEST STARTS ================\n", GetCurrentPid());

  inode = DfsInodeOpen("ece595-file-2");

  printf("runostests: ece595-file-2 open, inode = %d\n", inode);

  for (a = 0; a < NON_ALGIN_BYTE_AMOUNT; a++) {
    non_align_golden_data[a] = a;
  }

  for (a = 0; a < NUM_TESTCASE; a++) {
    DfsInodeWriteBytes(inode, non_align_golden_data, 256+a*NON_ALGIN_BYTE_AMOUNT, NON_ALGIN_BYTE_AMOUNT);
  }

  for (a = 0; a < NUM_TESTCASE; a++) {
    DfsInodeReadBytes(inode, non_align_data, 256+a*NON_ALGIN_BYTE_AMOUNT, NON_ALGIN_BYTE_AMOUNT);

    for (i = 0; i < NON_ALGIN_BYTE_AMOUNT; i++) {
      if (non_align_data[i] != non_align_golden_data[i]) {
        printf("runostests (%d): FAIL: TESTCASE %d failed. Details: \n", GetCurrentPid(), a);
        printf("runostests (%d): FAIL: index non_align_data[%d] != non_align_golden_data[%d] (%d != %d)\n", GetCurrentPid(), i, i, non_align_data[i], non_align_golden_data[i]);
        GracefulExit();
      } else {
        if (i == NON_ALGIN_BYTE_AMOUNT-1)
          printf("runostests (%d): TESTCASE %d MATCHED\n", GetCurrentPid(), a);
      }
    }
  }

  DfsInodeDelete(inode);

  printf("runostests (%d): ================= NON-BLOCK-ALIGN TEST PASSED! ================\n\n\n", GetCurrentPid());
}

