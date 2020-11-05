#include <iostream>
#include "libcurlwrapper.hpp"

std::atomic<size_t> nAsyncs{0};

int main() {
  using namespace libcurlwrapper;
  WWWLibCurlWrapper WWW{};

  std::thread t{[&WWW]() {
    auto cb = [&WWW](Request& req) {
      static std::mutex ioMtx{};
      ioMtx.lock();
      std::cout << "Status: " << req.response_status << std::endl;
      ioMtx.unlock();
      nAsyncs++;
    };
    WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
    WWW << std::packaged_task<void(Request&)>(cb);
    WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
    WWW << std::packaged_task<void(Request&)>(cb);
    WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
    WWW << std::packaged_task<void(Request&)>(cb);
    WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
    WWW << std::packaged_task<void(Request&)>(cb);
    WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
    WWW << std::packaged_task<void(Request&)>(cb);
    WWW << directive::asyncperform;

    WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
    WWW << std::packaged_task<void(Request&)>(cb);
    WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
    WWW << std::packaged_task<void(Request&)>(cb);
    WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
    WWW << std::packaged_task<void(Request&)>(cb);
    WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
    WWW << std::packaged_task<void(Request&)>(cb);
    WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
    WWW << std::packaged_task<void(Request&)>(cb);
    WWW << directive::asyncperform;

    WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
    WWW << std::packaged_task<void(Request&)>(cb);
    WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
    WWW << std::packaged_task<void(Request&)>(cb);
    WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
    WWW << std::packaged_task<void(Request&)>(cb);
    WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
    WWW << std::packaged_task<void(Request&)>(cb);
    WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
    WWW << std::packaged_task<void(Request&)>(cb);
    WWW << directive::asyncperform;
  }};
  t.join();
  // Wait all requests.
  while(nAsyncs != 15) std::this_thread::sleep_for(std::chrono::milliseconds(20));
  return (EXIT_SUCCESS);
}
