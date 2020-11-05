#include <iostream>
#include "libcurlwrapper.hpp"

int main() {
  using namespace libcurlwrapper;
  WWWLibCurlWrapper WWW{};
  // Start new request by << url("http://")
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  // Add a function to print result
  WWW << ([](Request& req) {
      std::cout << req.error << std::endl;
      std::cout << req.headers_out << std::endl;
      std::cout << req.body_out << std::endl;
    });
  // Add some headers
  WWW << header("Cookie", "mid=1; sid=4C7EACC1664E450573CDFC14182DE2AA;");
  WWW << useragent("WWWBot");
  WWW << user("guest", "guest");
  WWW << postdata("name=n1&value=v1");
  WWW << directive::http_2_0;
  // Perform request synchronously
  WWW << directive::syncperform;
  // Start a new request
  WWW << url("https://www.crawler-test.com/content/page_content_size/2055");
  // Add function for result
  WWW << ([](Request& req) {
      std::cout << req.error << std::endl;
      std::cout << req.headers_out << std::endl;
      std::cout << req.body_out << std::endl;
    });
  // Set limit for memory
  WWW << limitram(10'000'000, "out.txt");
  WWW << directive::http_2_0;
  // Perform request synchronously
  WWW << directive::syncperform;
  return (EXIT_SUCCESS);
}
