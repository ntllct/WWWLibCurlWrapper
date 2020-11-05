#include <iostream>
#include "libcurlwrapper.hpp"

using namespace libcurlwrapper;

WWWLibCurlWrapper WWW{};
WWWLibCurlWrapper WWW2{};
std::mutex ioMtx{};
std::atomic<size_t> nAsyncs{0};

void print(const std::string& str) {
  ioMtx.lock();
  std::cout << str << std::endl;
  ioMtx.unlock();
}

void test_sync() {
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << ([](Request& req) {
    print(req.headers_out + "\n" + req.body_out + "\n");
  });
  WWW << header("Cookie", "mid=1; sid=4C7EACC1664E450573CDFC14182DE2AA;");
  WWW << useragent("WWWBot");
  WWW << user("guest", "guest");
  WWW << postdata("name=n1&value=v1");
  WWW << directive::http_2_0;
  WWW << directive::syncperform;

  WWW << url("https://www.crawler-test.com/content/page_content_size/2055");
  WWW << ([](Request& req) { print(req.headers_out + "\n"); });
  WWW << limitram(10'000'000, "out.txt");
  WWW << directive::http_2_0;
  WWW << directive::syncperform;
}
void test_async() {
  auto cb = [](Request& req) {
    print(std::to_string(req.response_status));
    nAsyncs++;
  };

  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << cb;
  WWW2 << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW2 << cb;
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << cb;
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << cb;
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << cb;
  WWW << directive::asyncperform;
  WWW2 << directive::asyncperform;

  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << cb;
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << cb;
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << cb;
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << cb;
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << cb;
  WWW << directive::asyncperform;

  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << cb;
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << cb;
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << cb;
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << cb;
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << cb;
  WWW << directive::asyncperform;
}

int main() {
  test_sync();
  auto t1 = std::thread(test_sync);
  auto t2 = std::thread(test_async);
  t1.join();
  t2.join();
  while(nAsyncs != 15) std::this_thread::sleep_for(std::chrono::milliseconds(20));
  return (EXIT_SUCCESS);
}
