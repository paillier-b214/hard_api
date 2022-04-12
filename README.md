# Hardware API

## Usage

Move the whole repository into your project.
Assume $TARGET is your cmake target.

Add this code to your CMakeLists.txt:
```cmake
add_subdirectory(hard_api)
target_link_libraries($TARGET paillier_api)
```

Include the header in your cpp file.
```c++
#include "paillier/hard_api.h"
```

More detail see [`tests/main.cpp`](./tests/main.cpp).

## Credits

PYNQ API: https://github.com/mesham/pynq_api