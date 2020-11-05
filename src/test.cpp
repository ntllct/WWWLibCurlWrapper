#include "libcurlwrapper.hpp"

#include <iostream>

using namespace libcurlwrapper;

WWWLibCurlWrapper WWW{};

void show_test_name(const char* testname) {
  std::cout << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << testname << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << std::endl;
}
void wait_key() {
  std::cout << std::endl;
  std::cout << "Press ENTER to continue..." << std::endl;
  std::cin.get();
  std::cout << std::endl;
}

void test_useragent() {
  show_test_name("Test user-agent header");
  
  WWW << url("https://www.crawler-test.com/other/crawler_user_agent");
  WWW << useragent("WWWLibCurlWrapperBot");
  WWW << directive::verbose;  // Print headers in console
  WWW << directive::syncperform;
  
  wait_key();
}
void test_header() {
  show_test_name("Test arbitrary header");
  
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << header("Header", "Value");
  WWW << directive::verbose;  // Print headers in console
  WWW << directive::syncperform;
  
  wait_key();
}
void test_auth() {
  show_test_name("Test auth");
  
  WWW << url("https://www.crawler-test.com/other/basic_auth");
  // You will not see this in debug output
  WWW << user("guest", "qwerty");
  WWW << directive::verbose;  // Print headers in console
  WWW << directive::syncperform;
  
  wait_key();
}
void test_fileout() {
  show_test_name("Test fileout");
  
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << fileout("content.txt");
  WWW << directive::verbose;  // Print headers in console
  WWW << directive::syncperform;
  std::ifstream ifs("content.txt", std::ifstream::in);
  if (ifs) {
    std::cout << ifs.rdbuf() << std::endl;
    ifs.close();
  } else {
    std::cout << "Error!" << std::endl;
  }
  
  wait_key();
}
void test_uploadfile() {
  show_test_name("Test PUT request");
  
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << uploadfile("content.txt");
  WWW << directive::verbose;  // Print headers in console
  WWW << directive::syncperform;
  
  wait_key();
}
void test_mime() {
  show_test_name("Test POST request 1");
  
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << mimefile("filename", "content.txt");
  WWW << mimedata("name1", "value1");
  WWW << mimedata("name2", "value2");
  WWW << directive::verbose;  // Print headers in console
  WWW << directive::syncperform;
  
  wait_key();
}
void test_post() {
  show_test_name("Test POST request 2");
  
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << postdata("x=20&y=23");
  WWW << directive::verbose;  // Print headers in console
  WWW << directive::syncperform;
  
  wait_key();
}
void test_limitram() {
  show_test_name("Test limitram");
  
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << limitram(10'000'000, "content2.txt");
  WWW << directive::verbose;  // Print headers in console

  WWW << ([](Request& req) {
    if (!req.body_out.empty())
      std::cout << "IN RAM " << req.body_out.size() << std::endl;
    if (!req.filenameout.empty())
      std::cout << "IN FILE " << req.filenameout << std::endl;
  });
  WWW << directive::syncperform;

  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  WWW << limitram(1'000, "content2.txt");
  WWW << directive::verbose;  // Print headers in console

  WWW << ([](Request& req) {
    if (!req.body_out.empty())
      std::cout << "IN RAM " << req.body_out.size() << std::endl;
    if (!req.filenameout.empty())
      std::cout << "IN FILE " << req.filenameout << std::endl;
  });
  WWW << directive::syncperform;
  
  wait_key();
}
void print_request() {
  show_test_name("Print request");
  
  WWW << url("https://www.crawler-test.com/other/crawler_request_headers");
  int val = 100;
  WWW << userdata(&val);
  WWW << directive::verbose;  // Print headers in console
  WWW << ([](Request& req) {
    std::cout << "req.headers_out.size() = " << req.headers_out.size() << std::endl;
    std::cout << "req.body_out.size() = " << req.body_out.size() << std::endl;
    std::cout << "req.query_url = " << req.query_url << std::endl;
    std::cout << "req.effective_url = " << req.effective_url << std::endl;
    std::cout << "req.filenameout = " << req.filenameout << std::endl;
    std::cout << "req.error = " << req.error << std::endl;
    std::cout << "req.content_type = " << req.content_type << std::endl;
    std::cout << "req.user_data = " << req.user_data << std::endl;
    std::cout << "req.response_status = " << req.response_status << std::endl;
    std::cout << "req.file_time = " << req.file_time << std::endl;
    auto cookies = req.getCookies();
    for(const auto& c : cookies)
      std::cout << "Set cookie: " << c << std::endl;
  });
  WWW << directive::syncperform;

  wait_key();
}

int main() {
  test_useragent();
  test_header();
  test_auth();
  test_fileout();
  test_uploadfile();
  test_mime();
  test_post();
  test_limitram();
  print_request();
  return (EXIT_SUCCESS);
}
