/* http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine */
/* modified so it works :) */
#include <sys/sysctl.h>
#include <sys/types.h>
#include <stdio.h>

int main(void) {
  int mib[2];
  size_t len = 4;
  uint32_t numCPU;
  
  /* set the mib for hw.ncpu */
  mib[0] = CTL_HW;
  mib[1] = HW_AVAILCPU;

  /* get the number of CPUs from the system */
  sysctl(mib, 2, &numCPU, &len, NULL, 0);
  
  if (numCPU < 1) {
       mib[1] = HW_NCPU;
       sysctl(mib, 2, &numCPU, &len, NULL, 0);
       if (numCPU < 1) {
            numCPU = 1;
       }
  }
  
  printf("%i\n", numCPU);
}
