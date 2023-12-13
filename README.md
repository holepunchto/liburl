# liburl

WHATWG URL parser in C. Provides a memory efficient URL representation with immutable views over the various URL components, inspired by [WebKit](https://github.com/WebKit/WebKit) and [Ada](https://github.com/ada-url/ada).

## Usage

```c
#include <url.h>
#include <utf.h>

url_t url
url_init(&url);

url_parse(&url, "https://example.com/foo/bar?baz", -1, NULL);

utf8_string_view_t href = url_get_href(&url);

url_destroy(&url);
```

## API

See [`include/url.h`](include/url.h) for the public API.

## License

Apache-2.0
