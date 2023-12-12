# liburl

WHATWG URL parser in C.

## Usage

```c
#include <url.h>
#include <utf.h>

url_t url
url_init(&url);

int err = url_parse(&url, "https://example.com/foo/bar?baz", -1, NULL);
if (err < 0) abort();

utf8_string_view_t href = url_get_href(&url);

printf("href=%.*s\n", (int) href.len, href.data);

url_destroy(&url);
```

## API

See [`include/url.h`](include/url.h) for the public API.

## License

Apache-2.0
