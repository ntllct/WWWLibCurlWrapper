#pragma once

#include <curl/curl.h>
#include <fstream>
#include <future>
#include <thread>
#include <vector>

namespace libcurlwrapper {

struct header {
  const char* name{nullptr};
  const char* value{nullptr};
  header() = delete;
  header(const char* n, const char* v) : name(n), value(v) {}
};
struct user {
  const char* name{nullptr};
  const char* pass{nullptr};
  user() = delete;
  user(const char* n, const char* p) : name(n), pass(p) {}
};
struct url {
  const char* c_url{nullptr};
  url() = delete;
  url(const char* u) : c_url(u) {}
};
struct maxmultitotal {
  long max{-1};
  maxmultitotal() = delete;
  maxmultitotal(long m) : max(m) {}
};
struct maxmultihost {
  long max{-1};
  maxmultihost() = delete;
  maxmultihost(long m) : max(m) {}
};
struct maxconnections {
  long max{-1};
  maxconnections() = delete;
  maxconnections(long m) : max(m) {}
};
struct timeout {
  long val{-1};
  timeout() = delete;
  timeout(long m) : val(m) {}
};
struct connectiontimeout {
  long val{-1};
  connectiontimeout() = delete;
  connectiontimeout(long m) : val(m) {}
};
struct maxredirs {
  long max{-1};
  maxredirs() = delete;
  maxredirs(long m) : max(m) {}
};
struct useragent {
  const char* agent{nullptr};
  useragent() = delete;
  useragent(const char* ua) : agent(ua) {}
};
struct expect100timeout_ms {
  long val{-1};
  expect100timeout_ms() = delete;
  expect100timeout_ms(long m) : val(m) {}
};
struct userdata {
  void* data{nullptr};
  userdata() = delete;
  userdata(void* d) : data(d) {}
};
struct fileout {
  const char* filename{nullptr};
  fileout() = delete;
  fileout(const char* f) : filename(f) {}
};
struct limitram {
  long val{-1};
  const char* filename{nullptr};
  limitram() = delete;
  limitram(long m, const char* f) : val(m), filename(f) {}
};
struct postdata {
  std::string data{};
  postdata() = delete;
  postdata(const char* d) : data(d) {}
  postdata(std::string&& d) : data(std::forward<std::string>(d)) {}
};
struct uploadfile {  // PUT /
  const char* filename{nullptr};
  uploadfile() = delete;
  uploadfile(const char* f) : filename(f) {}
};
struct mimefile {
  const char* name{nullptr};
  const char* value{nullptr};
  mimefile() = delete;
  mimefile(const char* n, const char* v) : name(n), value(v) {}
};
struct mimedata {
  const char* name{nullptr};
  const char* value{nullptr};
  mimedata() = delete;
  mimedata(const char* n, const char* v) : name(n), value(v) {}
};

enum class directive : unsigned char {
  syncperform,
  asyncperform,
  http_none,
  http_1_0,
  http_1_1,
  http_2_0,
  http_2_0_tls,
  http_2_0_pk,
  skip_ssl_verify_peer,
  skip_ssl_verify_host,
  dontfollowlocation,
  verbose,
};

class WWWLibCurlWrapper;
struct Request {
  std::string query_url{};
  std::string effective_url{};
  std::string headers_out{};
  std::string body_out{};
  std::string filenameout{};
  std::string error{};
  std::string content_type{};  // for result
  void* user_data{nullptr};
  long response_status{0};
  long file_time{0};

  std::vector<std::string> getCookies() {
    CURLcode res_{};
    curl_slist* cookies_ = nullptr;
    curl_slist* nc_ = nullptr;
    res_ = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies_);
    std::vector<std::string> out{};
    if (res_ != CURLE_OK) return (out);
    nc_ = cookies_;
    while (nc_) {
      out.push_back(nc_->data);
      nc_ = nc_->next;
    }
    curl_slist_free_all(cookies_);
    return (out);
  }

  ~Request() {
    if (header_chunk != nullptr) curl_slist_free_all(header_chunk);
    if (curl != nullptr) curl_easy_cleanup(curl);
    if (fd != nullptr) fclose(fd);
    if (form != nullptr) curl_mime_free(form);
  }

  friend WWWLibCurlWrapper;

 private:
  std::string filenameout_when_limit_reached{};
  std::string filenamereal{};
  std::string post_data{};
  std::string user_agent{};
  std::string username{};
  std::string password{};

  CURLcode result{};

  std::vector<std::pair<std::string, std::string>> mimefiles{};
  std::vector<std::pair<std::string, std::string>> mimedatas{};
  std::vector<std::pair<std::string, std::string>> headers{};

  std::fstream f_out{};

  CURL* curl{nullptr};
  curl_slist* header_chunk{nullptr};
  curl_mime* form{nullptr};

  FILE* fd{nullptr};
  std::packaged_task<void(Request&)> callback{[](Request& req) {}};

  long ssl_verify_peer{1};
  long ssl_verify_host{1};
  long verbose{0};
  long connection_timeout{20};
  long time_out{15};
  long follow_location{1};
  long max_redirs{3};
  long timeout100{3000};
  long http_version{CURL_HTTP_VERSION_NONE};
  size_t max_body_size{1'000'000};
};

class WWWLibCurlWrapper {
 public:
  size_t globalSize() const noexcept { return (globalRequests.size()); }
  size_t localSize() const noexcept { return (localRequests.size()); }
  // class control
  WWWLibCurlWrapper& operator<<(const directive d) {
    if (localRequests.empty()) return (*this);
    switch (d) {
      case directive::syncperform:
        _perform(localRequests);
        localRequests.clear();
        break;
      case directive::asyncperform:
        mtxRequests.lock();
        for (auto& r : localRequests) globalRequests.push_back(std::move(r));
        mtxRequests.unlock();
        cV.notify_one();
        localRequests.clear();
        break;
      case directive::http_none:
        localRequests.back()->http_version = CURL_HTTP_VERSION_NONE;
        break;
      case directive::http_1_0:
        localRequests.back()->http_version = CURL_HTTP_VERSION_1_0;
        break;
      case directive::http_1_1:
        localRequests.back()->http_version = CURL_HTTP_VERSION_1_1;
        break;
      case directive::http_2_0:
        localRequests.back()->http_version = CURL_HTTP_VERSION_2_0;
        break;
      case directive::http_2_0_tls:
        localRequests.back()->http_version = CURL_HTTP_VERSION_2TLS;
        break;
      case directive::http_2_0_pk:
        localRequests.back()->http_version =
            CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE;
        break;
      case directive::skip_ssl_verify_peer:
        localRequests.back()->ssl_verify_peer = 0;
        break;
      case directive::skip_ssl_verify_host:
        localRequests.back()->ssl_verify_host = 0;
        break;
      case directive::dontfollowlocation:
        localRequests.back()->follow_location = 0;
        break;
      case directive::verbose:
        localRequests.back()->verbose = 1;
        break;
      default:
        break;
    }
    return (*this);
  }
  // New address
  WWWLibCurlWrapper& operator<<(const url& d) {
    localRequests.emplace_back(new Request);
    localRequests.back()->query_url.assign(d.c_url);
    return (*this);
  }
  // New header for all urls in session
  WWWLibCurlWrapper& operator<<(const header& h) {
    if (localRequests.empty()) return (*this);
    localRequests.back()->headers.emplace_back(h.name, h.value);
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(const user& u) {
    if (localRequests.empty()) return (*this);
    localRequests.back()->username.assign(u.name);
    localRequests.back()->password.assign(u.pass);
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(const maxredirs& mr) {
    if (localRequests.empty()) return (*this);
    localRequests.back()->max_redirs = mr.max;
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(const maxmultitotal& mm) {
    max_multi_total = mm.max;
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(const maxmultihost& mm) {
    max_multi_host = mm.max;
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(const maxconnections& mm) {
    max_connections = mm.max;
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(const connectiontimeout& mm) {
    if (localRequests.empty()) return (*this);
    localRequests.back()->connection_timeout = mm.val;
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(const timeout& mm) {
    if (localRequests.empty()) return (*this);
    localRequests.back()->time_out = mm.val;
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(const expect100timeout_ms& mm) {
    if (localRequests.empty()) return (*this);
    localRequests.back()->timeout100 = mm.val;
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(const useragent& ua) {
    if (localRequests.empty()) return (*this);
    localRequests.back()->user_agent.assign(ua.agent);
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(std::packaged_task<void(Request&)>&& cb) {
    if (localRequests.empty()) return (*this);
    localRequests.back()->callback =
        std::forward<std::packaged_task<void(Request&)>>(cb);
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(void (*cb)(Request&)) {
    if (localRequests.empty()) return (*this);
    localRequests.back()->callback = std::packaged_task<void(Request&)>(cb);
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(const userdata& data) {
    if (localRequests.empty()) return (*this);
    localRequests.back()->user_data = data.data;
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(const fileout& file) {
    if (localRequests.empty()) return (*this);
    localRequests.back()->filenameout.assign(file.filename);  // = uid.id;
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(const limitram& lr) {
    if (localRequests.empty()) return (*this);
    localRequests.back()->max_body_size = lr.val;
    localRequests.back()->filenameout_when_limit_reached.assign(lr.filename);
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(const postdata& data) {
    if (localRequests.empty()) return (*this);
    localRequests.back()->post_data = std::move(data.data);
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(const uploadfile& file) {
    if (localRequests.empty()) return (*this);
    if (localRequests.back()->fd != nullptr) fclose(localRequests.back()->fd);

    localRequests.back()->fd = fopen(file.filename, "rb");
    if (localRequests.back()->fd == nullptr) return (*this);

    if (filesize(file.filename) == -1) {
      fclose(localRequests.back()->fd);
      localRequests.back()->fd = nullptr;
      return (*this);
    }
    localRequests.back()->filenamereal = file.filename;
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(const mimefile& file) {
    if (localRequests.empty()) return (*this);
    localRequests.back()->mimefiles.emplace_back(file.name, file.value);
    return (*this);
  }
  WWWLibCurlWrapper& operator<<(const mimedata& data) {
    if (localRequests.empty()) return (*this);
    localRequests.back()->mimedatas.emplace_back(data.name, data.value);
    return (*this);
  }
  WWWLibCurlWrapper() {
    size_t old = nInstances++;
    if (old == 0) curl_global_init(CURL_GLOBAL_DEFAULT);
    run();
  }
  ~WWWLibCurlWrapper() {
    auto old = nInstances--;
    if (old == 1) curl_global_cleanup();
    stop();
  }

 private:
  // Join to thread
  void run() {
    bool needRun = !isRunning.exchange(true);
    if (!needRun) return;
    crawlerThread = std::thread(WWWLibCurlWrapper::_serve, this);
  }
  // Set is_running false
  void stop() {
    bool needJoin = isRunning.exchange(false);
    if(needJoin) {
      isRunning = false;
      cV.notify_all();
      if(crawlerThread.joinable())
        crawlerThread.join();
    }
  }
  
  std::ifstream::pos_type filesize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
  }
  static void _serve(WWWLibCurlWrapper* this_) {
    while (isRunning) {
      std::unique_lock<std::mutex> cv_lock_(cvMtx);
      cV.wait(cv_lock_,
              [this_] { return (!globalRequests.empty() || !isRunning); });

      if (!isRunning) break;

      if (globalRequests.empty()) continue;

      mtxRequests.lock();
      auto reqs = std::move(globalRequests);
      globalRequests.clear();
      mtxRequests.unlock();
      if (!reqs.empty()) this_->_perform(reqs);
    }
  }

  // Thread for async performs
  static inline std::thread crawlerThread{};
  // Local running flag
  static inline std::atomic<bool> isRunning{false};
  // How much instances of class created. For global init/cleanup.
  static inline std::atomic<size_t> nInstances{0};

  // Each thread has its own structure
  thread_local static inline std::vector<std::unique_ptr<Request>>
      localRequests{};
  // For async perform
  static inline std::vector<std::unique_ptr<Request>> globalRequests{};
  // For working with queue
  static inline std::mutex mtxRequests{};
  // For async queries
  static inline std::mutex cvMtx{};
  static inline std::condition_variable cV{};

  static inline long max_multi_total{20};
  static inline long max_multi_host{6};
  static inline long max_connections{20};

  void _perform(std::vector<std::unique_ptr<Request>>& reqs) {
    if (reqs.empty()) return;

    if (reqs.size() == 1)
      _perform_once(*reqs[0]);
    else
      _perform_multi(reqs);
  }
  void _process_result(Request& req) {
    if (req.result == CURLE_OK) {
      const char* url_ = "nullptr";
      curl_easy_getinfo(req.curl, CURLINFO_EFFECTIVE_URL, &url_);
      req.effective_url.assign(url_);
      long res_status_ = 0;
      curl_easy_getinfo(req.curl, CURLINFO_RESPONSE_CODE, &res_status_);
      req.response_status = res_status_;
      if (res_status_ == 200) {
        char* ctype_ = nullptr;
        curl_easy_getinfo(req.curl, CURLINFO_CONTENT_TYPE, &ctype_);
        req.content_type.assign(ctype_);
        curl_easy_getinfo(req.curl, CURLINFO_FILETIME, &req.file_time);
      }
    } else {
      req.error.assign("Connection failure!");
    }

    if (req.f_out.is_open()) req.f_out.close();

    req.callback(req);
  }
  void _perform_once(Request& req) {
    _make_handle(req, 0);
    req.result = curl_easy_perform(req.curl);
    _process_result(req);
  }
  void _perform_multi(std::vector<std::unique_ptr<Request>>& reqs) {
    CURLM* cm = curl_multi_init();
    curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, max_connections);
    curl_multi_setopt(cm, CURLMOPT_MAX_TOTAL_CONNECTIONS, max_multi_total);
    curl_multi_setopt(cm, CURLMOPT_MAX_HOST_CONNECTIONS, max_multi_host);
/* enables http/2 if available */
#ifdef CURLPIPE_MULTIPLEX
    curl_multi_setopt(cm, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);
#endif
    // Collect all curls
    for (size_t i = 0; i < reqs.size(); ++i)
      curl_multi_add_handle(cm, _make_handle(*reqs[i], i));

    int numfds_ = 0;
    int still_running_ = static_cast<int>(reqs.size());

    while (still_running_ > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(20));

      CURLMcode mc_;
      mc_ = curl_multi_wait(cm, nullptr, 0, 1000, &numfds_);
      if (mc_ != CURLM_OK) return;
      mc_ = curl_multi_perform(cm, &still_running_);
      if (mc_ != CURLM_OK) return;

      int msgs_left_ = -1;
      CURLMsg* m_ = nullptr;

      while ((m_ = curl_multi_info_read(cm, &msgs_left_))) {
        if (m_->msg == CURLMSG_DONE) {
          CURL* handle_ = m_->easy_handle;
          size_t url_index_ = 0;
          curl_easy_getinfo(handle_, CURLINFO_PRIVATE, &url_index_);
          reqs[url_index_]->result = m_->data.result;
          _process_result(*reqs[url_index_]);
          curl_multi_remove_handle(cm, handle_);
        }
      }
    }
    curl_multi_cleanup(cm);
  }
  //
  CURL* _make_handle(Request& req, size_t id) {
    int (*callback_header_)(char*, size_t, size_t, Request*) =
        [](char* data, size_t size, size_t nmemb, Request* ocd) -> int {
      if (ocd == nullptr) return (0);
      if (data == nullptr) return (0);
      // Header length limit
      if (ocd->headers_out.size() < 10000)
        ocd->headers_out.append(data, size * nmemb);

      return (size * nmemb);
    };
    int (*callback_body_)(char*, size_t, size_t, Request*) =
        [](char* data, size_t size, size_t nmemb, Request* ocd) -> int {
      if (ocd == nullptr) return (0);
      if (data == nullptr) return (0);
      if (!ocd->filenameout.empty()) {
        if (!ocd->error.empty()) return (size * nmemb);
        // Open file if necessary
        if (!ocd->f_out.is_open() && !ocd->f_out.fail())
          ocd->f_out.open(ocd->filenameout.c_str(),
                          std::ios::out | std::ios::trunc);
        // Write to file
        if (ocd->f_out.is_open()) {
          ocd->f_out.write(data, size * nmemb);
        } else {
          ocd->error.assign("Cannot open file: ");
          ocd->error.append(ocd->filenameout);
        }
      } else {
        if (ocd->body_out.size() <= ocd->max_body_size)
          ocd->body_out.append(data, size * nmemb);
        if (ocd->body_out.size() > ocd->max_body_size) {
          if (!ocd->filenameout_when_limit_reached.empty()) {
            ocd->filenameout.swap(ocd->filenameout_when_limit_reached);
            // Open file if necessary
            if (!ocd->f_out.is_open() && !ocd->f_out.fail())
              ocd->f_out.open(ocd->filenameout.c_str(),
                              std::ios::out | std::ios::trunc);
            // Write to file
            if (ocd->f_out.is_open()) {
              ocd->f_out.write(ocd->body_out.c_str(), ocd->body_out.size());
              ocd->body_out.clear();
              ocd->body_out.shrink_to_fit();
            } else {
              ocd->error.assign("Cannot open file: ");
              ocd->error.append(ocd->filenameout);
            }
          }
        }
      }

      return (size * nmemb);
    };

    req.curl = curl_easy_init();

    curl_easy_setopt(req.curl, CURLOPT_HTTP_VERSION, req.http_version);
    curl_easy_setopt(req.curl, CURLOPT_URL, req.query_url.c_str());
    curl_easy_setopt(req.curl, CURLOPT_PRIVATE, id);
    curl_easy_setopt(req.curl, CURLOPT_WRITEDATA, &req);
    curl_easy_setopt(req.curl, CURLOPT_WRITEFUNCTION, callback_body_);
    curl_easy_setopt(req.curl, CURLOPT_HEADERDATA, &req);
    curl_easy_setopt(req.curl, CURLOPT_HEADERFUNCTION, callback_header_);
    curl_easy_setopt(req.curl, CURLOPT_SSL_VERIFYPEER, req.ssl_verify_peer);
    curl_easy_setopt(req.curl, CURLOPT_SSL_VERIFYHOST, req.ssl_verify_host);
    curl_easy_setopt(req.curl, CURLOPT_CONNECTTIMEOUT, req.connection_timeout);
    curl_easy_setopt(req.curl, CURLOPT_TIMEOUT, req.time_out);
    curl_easy_setopt(req.curl, CURLOPT_FOLLOWLOCATION, req.follow_location);
    curl_easy_setopt(req.curl, CURLOPT_MAXREDIRS, req.max_redirs);
    curl_easy_setopt(req.curl, CURLOPT_FILETIME, 1L);
    curl_easy_setopt(req.curl, CURLOPT_USERAGENT, req.user_agent.c_str());
    curl_easy_setopt(req.curl, CURLOPT_EXPECT_100_TIMEOUT_MS, req.timeout100);
    curl_easy_setopt(req.curl, CURLOPT_UNRESTRICTED_AUTH, 1L);
    curl_easy_setopt(req.curl, CURLOPT_VERBOSE, req.verbose);
    curl_easy_setopt(req.curl, CURLOPT_ACCEPT_ENCODING,
                     "");  // automatic decompression
    // curl_easy_setopt(req.curl, CURLOPT_COOKIEFILE, "");
    // curl_easy_setopt(req.curl, CURLOPT_COOKIELIST, "ALL");
    // curl_easy_setopt(req.curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);

    if (!req.username.empty()) {
      std::string auth(req.username.size() + req.password.size() + 1, 0);
      auth.append(req.username);
      auth.push_back(':');
      auth.append(req.password);
      curl_easy_setopt(req.curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
      curl_easy_setopt(req.curl, CURLOPT_USERPWD, auth.c_str());
    }

    // Append raw POST data
    if (!req.post_data.empty()) {
      curl_easy_setopt(req.curl, CURLOPT_POSTFIELDS, req.post_data.c_str());
      curl_easy_setopt(req.curl, CURLOPT_POSTFIELDSIZE,
                       static_cast<long>(req.post_data.size()));
    }
    // PUT file
    if (req.fd != nullptr) {
      // tell it to "upload" to the URL
      curl_easy_setopt(req.curl, CURLOPT_UPLOAD, 1L);
      // set where to read from (on Windows you need to use READFUNCTION too)
      curl_easy_setopt(req.curl, CURLOPT_READDATA, req.fd);
      // and give the size of the upload (optional)
      curl_easy_setopt(
          req.curl, CURLOPT_INFILESIZE_LARGE,
          static_cast<curl_off_t>(filesize(req.filenamereal.c_str())));
    }
    // Collect headers
    if (!req.headers.empty()) {
      for (const auto& [name, value] : req.headers) {
        if (name.empty()) continue;
        std::string line_(name);
        if (line_.back() != ':') line_.push_back(':');
        if (!value.empty()) {
          line_.push_back(' ');
          line_.append(value);
        }
        req.header_chunk = curl_slist_append(req.header_chunk, line_.c_str());
      }
      curl_easy_setopt(req.curl, CURLOPT_HTTPHEADER, req.header_chunk);
    }
    // For mime POST
    if (!req.mimefiles.empty() || !req.mimedatas.empty()) {
      //
      req.form = curl_mime_init(req.curl);
      curl_mimepart* field = nullptr;
      // Collect files
      for (const auto& [name, file] : req.mimefiles) {
        field = curl_mime_addpart(req.form);
        curl_mime_name(field, name.c_str());
        curl_mime_filedata(field, file.c_str());
      }
      // Collect mime vars
      for (const auto& [name, value] : req.mimedatas) {
        field = curl_mime_addpart(req.form);
        curl_mime_name(field, name.c_str());
        curl_mime_data(field, value.c_str(), CURL_ZERO_TERMINATED);
      }

      curl_easy_setopt(req.curl, CURLOPT_MIMEPOST, req.form);
    }
    return (req.curl);
  }
};

}  // namespace libcurlwrapper
