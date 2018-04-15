#include "minunit.h"

MU_TEST(test_traceroute) {
  // TODO: Test various core components of the traceroute program.
  // Testing parts of traceroute are hard without the right tools,
  // especially because the program interacts with the kernel
  // and the network.
  // I have decided at the present moment _not_ to invest
  // in researching or building those tools.
  // The goal of this program was not to be an alternative to
  // the traceroute packaged with the OS, but to provide a learning
  // experience.
}

MU_TEST_SUITE(test_suite) {
}

int main() {
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  return minunit_status;
}
