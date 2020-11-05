### Start a new request with URL:
```
WWW << url("http://www.example.com");
```

After this, you can add headers or other information for this url. After new `WWW << url("http://www.example.com");` you cannot edit previous request.

### Add arbitrary HTTP header:
```
WWW << header("Name", "Value");
```

### Add login and password for HTTP request:
```
WWW << user("guest", "qwerty");
```

### Set max simultaneously open connections:
```
WWW << maxmultitotal(10);
```

### Set max number of connections to a single host:
```
WWW << maxmultihost(10);
```

### Set size of connection cache:
```
WWW << maxconnections(10);
```

### Set maximum time the request is allowed to take:
```
WWW << timeout(10);
```

### Set timeout for the connect phase:
```
WWW << connectiontimeout(10);
```

### Set maximum number of redirects allowed:
```
WWW << maxredirs(3);
```

### Set user agent header:
```
WWW << useragent("WWWLibCurlWrapperBot");
```

### Set timeout for "Expect: 100-continue" response:
```
WWW << expect100timeout_ms(10000);
```

### Save response to file:
```
WWW << fileout("result.txt");
```

### Set RAM memory limit per request in bytes:
```
WWW << limitram(10000000, "result.txt");
```
If the limit is reached, all data will be saved into file. Check `if (!req.filenameout.empty())` if it happened.

### Post url encoded data:
```
WWW << postdata("x=20&y=12");
```

### Upload file by PUT request:
```
WWW << uploadfile("filename.txt");
```

### Upload files by mime POST request:
```
WWW << mimefile("name1", "filename1.txt");
WWW << mimefile("name2", "filename2.txt");
```

### POST data by mime POST request:
```
WWW << mimedata("name1", "value1");
WWW << mimedata("name2", "value1");
```

### Pass user data into request to recognize in callback function:
```
int val = 1234;
WWW << userdata(&val);
```


## Directives

### Make synchronus request:
```
WWW << directive::syncperform;
```
The thread will be blocked until a callback function is finished.

### Make asynchronus request:
```
WWW << directive::asyncperform;
```
The thread won't be blocked. A callback function will be called from a WWWLibCurlWrapper maintenance thread.

### Set HTTP type:
```
WWW << directive::http_none;
```
```
WWW << directive::http_1_0;
```
```
WWW << directive::http_1_1;
```
```
WWW << directive::http_2_0;
```
```
WWW << directive::http_2_0_tls;
```
```
WWW << directive::http_2_0_pk;
```

### Don't verify SSL:
```
WWW << directive::skip_ssl_verify_peer;
```
```
WWW << directive::skip_ssl_verify_host;
```

### Don't allow redirects:
```
WWW << directive::dontfollowlocation;
```

### Debug mode. You will see all requests in console. This is very useful for debugging and understanding problems.
```
WWW << directive::verbose;
```

## Callback function

You can provide callback function like this:
```
WWW << url("https://www.example.com");
WWW << ([](Request& req) {
    std::cout << req.headers_out << std::endl;
    std::cout << req.body_out << std::endl;
  });
```
or
```
auto cb = [&WWW](Request& req) {
  static std::mutex ioMtx{};
  ioMtx.lock();
  std::cout << "Status: " << req.response_status << std::endl;
  ioMtx.unlock();
};
WWW << url("https://www.example.com");
WWW << std::packaged_task<void(Request&)>(cb);
```
or
```
void cb(Request& req) {
    std::cout << req.headers_out << std::endl;
    std::cout << req.body_out << std::endl;
}

...

WWW << url("https://www.example.com");
WWW << cb;
```

### The `Request` contains few data fields:

`req.query_url` - url, that you have provided by `url("https://www.example.com")`;\
`req.effective_url` - url, that you have redirected by "Location:" header;\
`headers_out` - response headers;\
`body_out` - response content;\
`filenameout` - filename with content;\
`error` - error message;\
`content_type` - content type;\
`user_data` - pointer to user data;\
`response_status` - status code;\
`file_time` - the remote time of the retrieved document;\
`getCookies()` - set cookies from response headers;\














