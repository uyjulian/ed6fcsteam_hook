
#include <stdlib.h>

extern "C" void __std_terminate() {
	exit(0);
}
