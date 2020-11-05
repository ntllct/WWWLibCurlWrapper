# WWWLibCurlWrapper

Easy to use asynchronous wrapper around libcurl.
This library allows you:
* You can make GET requests.
* You can make PUT requests.
* You can POST data and files.
* You can set memory limitation and save big data to file if the limit is reached.
* You can make synchronous and asynchronous requests.
* You can make multi requests at the same time.

Example:
```
#include <iostream>
#include "libcurlwrapper.hpp"

int main() {
  using namespace libcurlwrapper;
  WWWLibCurlWrapper WWW{};

  // Start a new request
  WWW << url("https://www.crawler-test.com/content/page_content_size/2055");

  // Add a function for result
  WWW << ([](Request& req) {
    std::cout << req.headers_out << std::endl;
    std::cout << req.body_out << std::endl;
  });
  
  // Make a request and wait for result
  WWW << directive::syncperform;
  
  return (EXIT_SUCCESS);
}
```
Don't forget to add libcurl and pthread to your compiler.
```
g++ -std=c++17 -m64 -O3 -mavx -Wall -pedantic-errors -Wold-style-cast -Weffc++ main.cpp -o main -lpthread -lcurl
```

## Build examples:
```
mkdir build
cd build
cmake ..
make
```

Run test with:
```
./LibCurlWrapper
```

Run examples with:
```
./example_sync
./example_async
./example_combined
```

For more information see:

[Example 1](examples/example_sync.cpp)\
[Example 2](examples/example_async.cpp)\
[Example 3](examples/example_combined.cpp)\
[Example 4](src/test.cpp)\

[API](README_API.md)

