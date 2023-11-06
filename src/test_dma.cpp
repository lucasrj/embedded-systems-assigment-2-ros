#include <errno.h>
#include <fcntl.h>
#include <iterator>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
// #include <stdlib.h>
#include <iostream>

#include <chrono>
#include <time.h>

#include <math.h>

#include "reserved_mem.hpp"
// #include <axi_dma_controller.h>
#include "axidma.h"
#include <xexample.h>

#define DEVICE_FILENAME "/dev/reservedmemLKM"
#define LENGTH 100
#define LENGTH_bits 100 * 32

// #define LENGTH 0x007fffff // Length in bytes
#define P_START 0x70000000
#define P_OFFSET 0x00001000

//#define i_P_START 0
//#define i_LENGTH 1
//#define i_U_BUFFER_PTR_L 2
//#define i_U_BUFFER_PTR_H 3

#define UIO_DMA_N 0

// clock_t t;
std::chrono::_V2::system_clock::time_point t1;
void start_timer() {
  t1 = std::chrono::high_resolution_clock::now();
  // std::cout << "Start timer" << std::endl;
}
double stop_timer() {
  auto t2 = std::chrono::high_resolution_clock::now();
  auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
  std::chrono::duration<double, std::milli> ms_double = t2 - t1;
  // std::cout << "Duration: " << ms_double.count() << "ms [" << (float)LENGTH /
  // 1000000. << "MB]" << std::endl;
  return ms_double.count();
}

void print_mem(void *virtual_address, int byte_count) {
  char *data_ptr = (char *)virtual_address;

  for (int i = 0; i < byte_count; i++) {
    printf("%02X", data_ptr[i]);

    // print a space every 4 bytes (0 indexed)
    if (i % 4 == 3) {
      printf(" ");
    }
  }

  printf("\n");
}

int main() {
  int ret = 0;
  double total_t = 0;
  double tmp = 0;
  printf("\nHello World! - Running DMA transfer test application with "
         "specified memory.\n\n");

  Reserved_Mem pmem;
  // AXIDMAController dma(UIO_DMA_N, 0x1000);
  axiDma dma(UIO_DMA_N, 0x1000);


  dma.IntrDisable(IRQ_ALL,DMA_DIR_DEVICE_TO_DMA);
  dma.IntrDisable(IRQ_ALL,DMA_DIR_DMA_TO_DEVICE);
  // XExample example;

  // int rc;
  // if ((rc = XExample_Initialize(&example, "example")) != XST_SUCCESS) {
  //   fprintf(stderr, "Initialization failed. Return code: %d\n", rc);
  //   exit(EXIT_FAILURE);
  // }

  // XExample_Release(&example);

  // if ((rc = XExample_Initialize(&example, "example")) != XST_SUCCESS) {
  //   fprintf(stderr, "Initialization failed. Return code: %d\n", rc);
  //   exit(EXIT_FAILURE);
  // }

  printf("Initialization successful.\n");
  // XExample_InterruptGlobalDisable(&example);
  // XExample_InterruptDisable(&example,1);

  //   XExample_DisableAutoRestart(&example);

  // u32 is_idle = XExample_IsReady(&example);

  // printf("watings for ip to be ready \n");
  // while(!XExample_IsReady(&example));
  // printf("is_ready =\n");

  int32_t *tx_buff = (int32_t *)malloc(LENGTH_bits);
  if (tx_buff == NULL) {
    printf("could not allocate user buffer\n");
    return -1;
  }

  int32_t *rx_buff = (int32_t *)malloc(LENGTH_bits);
  if (rx_buff == NULL) {
    printf("could not allocate user buffer\n");
    return -1;
  }

  for (int i = 0; i < LENGTH; i++)
    tx_buff[i] = i;

  printf("User memory reserved and filled\n");

  tmp = 0;
  start_timer();
  // ret = write(reserved_mem_fd, write_info_LKM, sizeof(write_info_LKM));
  pmem.transfer(tx_buff, 0, LENGTH_bits);
  total_t += stop_timer();
  std::cout << "Data transfered to reserved memory: " << total_t << "ms ["
            << (float)LENGTH_bits / 1000000. << "MB]" << std::endl;

  start_timer();

  printf("staring tranfur from %x to %x \r\n",P_START,P_START+P_OFFSET);

  int status = dma.SimpleTransfer((UINTPTR)(P_START + P_OFFSET), 3200,
                                  DMA_DIR_DEVICE_TO_DMA);
  if (status != 0) {
    printf("filed to open resive %i", status);
    return 1;
  }

  if (dma.SimpleTransfer((UINTPTR)(P_START), 3200, DMA_DIR_DMA_TO_DEVICE) !=
      0) {
    printf("filed to open sending");
    return 1;
  }

  printf("sending and reading data\r\n");

  while (
      ((dma.Busy(DMA_DIR_DEVICE_TO_DMA)) | (dma.Busy(DMA_DIR_DMA_TO_DEVICE)))) {
    printf("wating \r\n");
  }

  tmp = stop_timer();
  total_t += tmp;
  std::cout << "\nDMA setup done, transfer begun: " << tmp << "ms ["
            << (float)LENGTH_bits / 1000000. << "MB]\n"
            << std::endl;

  start_timer();
  // std::cout << "running example" << std::endl;
  // bool first = true;
  // XExample_Start(&example);

  //  while (!XExample_IsIdle(&example));
  std::cout << "...Waiting for MM2S synchronization...\n" << std::endl;

  // while (!dma.MM2SIsSynced()) {
  //   // if (first)
  //   // {
  //   // 	printf("Not synced yet...\n");
  //   // 	first = false;
  //   // }
  // }

  std::cout << "...Waiting for S2MM synchronization...\n" << std::endl;

  // while (!dma.S2MMIsSynced());

  // XExample_Start(&example);

  // std::cout << "interrupt " <<XExample_InterruptGetStatus(&example) <<
  // std::endl; std::cout << "ideal " << XExample_IsIdle(&example) << std::endl;
  // std::cout << "done " << XExample_IsDone(&example) <<  std::endl;
  // std::cout << "ready " << XExample_IsReady(&example) <<  std::endl;

  // while(!XExample_IsDone(&example));

  tmp = stop_timer();
  total_t += tmp;
  std::cout << "\nData transfered to transfered by DMA: " << tmp << "ms ["
            << (float)LENGTH_bits / 1000000. << "MB]\n"
            << std::endl;

  printf("ALL DONE!\n");

  std::cout << "Total duration of transfer: " << total_t << "ms ["
            << (float)LENGTH_bits / 1000000. << "MB]" << std::endl;

  pmem.gather(rx_buff, P_OFFSET, LENGTH);

  for (int i = 0; i < LENGTH / sizeof(int32_t); i++)
    std::cout << tx_buff[i] << "/" << rx_buff[i] << std::endl;
  return ret;
}
