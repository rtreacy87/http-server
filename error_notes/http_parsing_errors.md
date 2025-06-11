
```bash
make
gcc -Wall -Wextra -std=c99 -Iinclude -o server src/server.c src/http_parser.c src/parse_http_request_supplement.c src/send_http_response_supplement.c src/router.c
src/server.c: In function ‘handle_method_not_allowed’:
src/server.c:16:22: warning: implicit declaration of function ‘strdup’; did you mean ‘strcmp’? [-Wimplicit-function-declaration]
   16 |     response->body = strdup("Method not allowed");
      |                      ^~~~~~
      |                      strcmp
src/server.c:16:20: warning: assignment to ‘char *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
   16 |     response->body = strdup("Method not allowed");
      |                    ^
src/server.c: In function ‘handle_bad_request’:
src/server.c:25:20: warning: assignment to ‘char *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
   25 |     response->body = strdup("Bad request");
      |                    ^
src/server.c: In function ‘main’:
src/server.c:72:10: warning: unused variable ‘buffer’ [-Wunused-variable]
   72 |     char buffer[BUFFER_SIZE];
      |          ^~~~~~
src/send_http_response_supplement.c: In function ‘add_headers’:
src/send_http_response_supplement.c:20:4: warning: implicit declaration of function ‘update_headers’; did you mean ‘add_headers’? [-Wimplicit-function-declaration]
   20 |    update_headers(client_fd, response);
      |    ^~~~~~~~~~~~~~
      |    add_headers
src/send_http_response_supplement.c:21:4: warning: implicit declaration of function ‘update_content_length’ [-Wimplicit-function-declaration]
   21 |    update_content_length(client_fd, response);
      |    ^~~~~~~~~~~~~~~~~~~~~
src/send_http_response_supplement.c: At top level:
src/send_http_response_supplement.c:26:6: warning: conflicting types for ‘update_headers’; have ‘void(int,  http_response_t *)’
   26 | void update_headers(int client_fd, http_response_t* response) {
      |      ^~~~~~~~~~~~~~
src/send_http_response_supplement.c:20:4: note: previous implicit declaration of ‘update_headers’ with type ‘void(int,  http_response_t *)’
   20 |    update_headers(client_fd, response);
      |    ^~~~~~~~~~~~~~
src/send_http_response_supplement.c:36:6: warning: conflicting types for ‘update_content_length’; have ‘void(int,  http_response_t *)’
   36 | void update_content_length(int client_fd, http_response_t* response) {
      |      ^~~~~~~~~~~~~~~~~~~~~
src/send_http_response_supplement.c:21:4: note: previous implicit declaration of ‘update_content_length’ with type ‘void(int,  http_response_t *)’
   21 |    update_content_length(client_fd, response);
      |    ^~~~~~~~~~~~~~~~~~~~~
src/send_http_response_supplement.c: In function ‘update_headers’:
src/send_http_response_supplement.c:30:59: warning: ‘
   ’ directive output may be truncated writing 2 bytes into a region of size between 0 and 510 [-Wformat-truncation=]
   30 |         snprintf(header_line, sizeof(header_line), "%s: %s\r\n",
      |                                                           ^~~~
src/send_http_response_supplement.c:30:9: note: ‘snprintf’ output between 5 and 515 bytes into a destination of size 512
   30 |         snprintf(header_line, sizeof(header_line), "%s: %s\r\n",
      |         ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   31 |                 response->headers[i][0], response->headers[i][1]);
      |                 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
src/router.c: In function ‘handle_home_page’:
src/router.c:16:22: warning: implicit declaration of function ‘strdup’; did you mean ‘strcmp’? [-Wimplicit-function-declaration]
   16 |     response->body = strdup("<html><body><h1>Welcome to our HTTP Server!</h1></body></html>");
      |                      ^~~~~~
      |                      strcmp
src/router.c:16:20: warning: assignment to ‘char *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
   16 |     response->body = strdup("<html><body><h1>Welcome to our HTTP Server!</h1></body></html>");
      |                    ^
src/router.c: In function ‘handle_hello_page’:
src/router.c:26:20: warning: assignment to ‘char *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
   26 |     response->body = strdup("Hello, World!");
      |                    ^
src/router.c: In function ‘handle_not_found’:
src/router.c:35:20: warning: assignment to ‘char *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
   35 |     response->body = strdup("Page not found");
      |                    ^
```



```bash
make
gcc -Wall -Wextra -std=c99 -Iinclude -o server src/server.c src/http_parser.c src/parse_http_request_supplement.c src/send_http_response_supplement.c src/router.c
src/send_http_response_supplement.c: In function ‘update_headers’:
src/send_http_response_supplement.c:11:59: warning: ‘
   ’ directive output may be truncated writing 2 bytes into a region of size between 0 and 510 [-Wformat-truncation=]
   11 |         snprintf(header_line, sizeof(header_line), "%s: %s\r\n",
      |                                                           ^~~~
src/send_http_response_supplement.c:11:9: note: ‘snprintf’ output between 5 and 515 bytes into a destination of size 512
   11 |         snprintf(header_line, sizeof(header_line), "%s: %s\r\n",
      |         ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   12 |                 response->headers[i][0], response->headers[i][1]);
      |                 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
src/router.c: In function ‘handle_home_page’:
src/router.c:16:22: warning: implicit declaration of function ‘strdup’; did you mean ‘strcmp’? [-Wimplicit-function-declaration]
   16 |     response->body = strdup("<html><body><h1>Welcome to our HTTP Server!</h1></body></html>");
      |                      ^~~~~~
      |                      strcmp
src/router.c:16:20: warning: assignment to ‘char *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
   16 |     response->body = strdup("<html><body><h1>Welcome to our HTTP Server!</h1></body></html>");
      |                    ^
src/router.c: In function ‘handle_hello_page’:
src/router.c:26:20: warning: assignment to ‘char *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
   26 |     response->body = strdup("Hello, World!");
      |                    ^
src/router.c: In function ‘handle_not_found’:
src/router.c:35:20: warning: assignment to ‘char *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
   35 |     response->body = strdup("Page not found");
      |                    ^
```



```bash
 make
gcc -Wall -Wextra -std=c99 -Iinclude -o server src/server.c src/http_parser.c src/parse_http_request_supplement.c src/send_http_response_supplement.c src/router.c
src/router.c: In function ‘handle_home_page’:
src/router.c:16:22: warning: implicit declaration of function ‘strdup’; did you mean ‘strcmp’? [-Wimplicit-function-declaration]
   16 |     response->body = strdup("<html><body><h1>Welcome to our HTTP Server!</h1></body></html>");
      |                      ^~~~~~
      |                      strcmp
src/router.c:16:20: warning: assignment to ‘char *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
   16 |     response->body = strdup("<html><body><h1>Welcome to our HTTP Server!</h1></body></html>");
      |                    ^
src/router.c: In function ‘handle_hello_page’:
src/router.c:26:20: warning: assignment to ‘char *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
   26 |     response->body = strdup("Hello, World!");
      |                    ^
src/router.c: In function ‘handle_not_found’:
src/router.c:35:20: warning: assignment to ‘char *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
   35 |     response->body = strdup("Page not found");
      |                    ^
```

```bash
curl http://localhost:8080/hello
<!DOCTYPE html>
<html lang="en">
        <head>
                <meta charset="utf-8" />
                <link rel="icon" href="/favicon.png" />
                <link rel="manifest" href="/manifest.json" crossorigin="use-credentials" />
                <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1" />
                <meta name="robots" content="noindex,nofollow" />
                <link
                        rel="search"
                        type="application/opensearchdescription+xml"
                        title="Open WebUI"
                        href="/opensearch.xml"
                />

                <script>
                        // On page load or when changing themes, best to add inline in `head` to avoid FOUC
                        (() => {
                                if (localStorage?.theme && localStorage?.theme.includes('oled')) {
                                        document.documentElement.style.setProperty('--color-gray-900', '#000000');
                                        document.documentElement.style.setProperty('--color-gray-950', '#000000');
                                        document.documentElement.classList.add('dark');
                                } else if (
                                        localStorage.theme === 'light' ||
                                        (!('theme' in localStorage) && window.matchMedia('(prefers-color-scheme: light)').matches)
                                ) {
                                        document.documentElement.classList.add('light');
                                } else if (localStorage.theme && localStorage.theme !== 'system') {
                                        localStorage.theme.split(' ').forEach((e) => {
                                                document.documentElement.classList.add(e);
                                        });
                                } else if (localStorage.theme && localStorage.theme === 'system') {
                                        systemTheme = window.matchMedia('(prefers-color-scheme: dark)').matches;
                                        document.documentElement.classList.add(systemTheme ? 'dark' : 'light');
                                } else if (localStorage.theme && localStorage.theme === 'her') {
                                        document.documentElement.classList.add('dark');
                                        document.documentElement.classList.add('her');
                                } else {
                                        document.documentElement.classList.add('dark');
                                }

                                window.matchMedia('(prefers-color-scheme: dark)').addListener((e) => {
                                        if (localStorage.theme === 'system') {
                                                if (e.matches) {
                                                        document.documentElement.classList.add('dark');
                                                        document.documentElement.classList.remove('light');
                                                } else {
                                                        document.documentElement.classList.add('light');
                                                        document.documentElement.classList.remove('dark');
                                                }
                                        }
                                });
                        })();
                </script>

                <title>Open WebUI</title>


                <link rel="modulepreload" href="/_app/immutable/entry/start.d1188aec.js">
                <link rel="modulepreload" href="/_app/immutable/chunks/scheduler.8ceb707f.js">
                <link rel="modulepreload" href="/_app/immutable/chunks/singletons.4fbad8ba.js">
                <link rel="modulepreload" href="/_app/immutable/chunks/index.e51049e7.js">
                <link rel="modulepreload" href="/_app/immutable/entry/app.6765b7cb.js">
                <link rel="modulepreload" href="/_app/immutable/chunks/preload-helper.a4192956.js">
                <link rel="modulepreload" href="/_app/immutable/chunks/index.07e72a31.js">
        </head>
        <body data-sveltekit-preload-data="hover">
                <div style="display: contents">
                        <script>
                                {
                                        __sveltekit_ufogfo = {
                                                base: "",
                                                env: {}
                                        };

                                        const element = document.currentScript.parentElement;

                                        Promise.all([
                                                import("/_app/immutable/entry/start.d1188aec.js"),
                                                import("/_app/immutable/entry/app.6765b7cb.js")
                                        ]).then(([kit, app]) => {
                                                kit.start(app, element);
                                        });
                                }
                        </script>
                </div>

                <div
                        id="splash-screen"
                        style="position: fixed; z-index: 100; top: 0; left: 0; width: 100%; height: 100%"
                >
                        <style type="text/css" nonce="">
                                html {
                                        overflow-y: scroll !important;
                                }
                        </style>

                        <img
                                id="logo"
                                style="
                                        position: absolute;
                                        width: 6rem;
                                        height: 6rem;
                                        top: 41%;
                                        left: 50%;
                                        margin-left: -3rem;
                                "
                                src="/logo.svg"
                        />

                        <div
                                style="
                                        position: absolute;
                                        top: 33%;
                                        left: 50%;

                                        width: 24rem;
                                        margin-left: -12rem;

                                        display: flex;
                                        flex-direction: column;
                                        align-items: center;
                                "
                        >
                                <img
                                        id="logo-her"
                                        style="width: 13rem; height: 13rem"
                                        src="/logo.svg"
                                        class="animate-pulse-fast"
                                />

                                <div style="position: relative; width: 24rem; margin-top: 0.5rem">
                                        <div
                                                id="progress-background"
                                                style="
                                                        position: absolute;
                                                        width: 100%;
                                                        height: 0.75rem;

                                                        border-radius: 9999px;
                                                        background-color: #fafafa9a;
                                                "
                                        ></div>

                                        <div
                                                id="progress-bar"
                                                style="
                                                        position: absolute;
                                                        width: 0%;
                                                        height: 0.75rem;
                                                        border-radius: 9999px;
                                                        background-color: #fff;
                                                "
                                                class="bg-white"
                                        ></div>
                                </div>
                        </div>

                        <!-- <span style="position: absolute; bottom: 32px; left: 50%; margin: -36px 0 0 -36px">
                                Footer content
                        </span> -->
                </div>
        </body>
</html>

<style type="text/css" nonce="">
        html {
                overflow-y: hidden !important;
        }

        #splash-screen {
                background: #fff;
        }
        html.dark #splash-screen {
                background: #000;
        }

        html.dark #splash-screen img {
                filter: invert(1);
        }

        html.her #splash-screen {
                background: #983724;
        }

        #logo-her {
                display: none;
        }

        #progress-background {
                display: none;
        }

        #progress-bar {
                display: none;
        }

        html.her #logo {
                display: none;
        }

        html.her #logo-her {
                display: block;
                filter: invert(1);
        }

        html.her #progress-background {
                display: block;
        }

        html.her #progress-bar {
                display: block;
        }

        @media (max-width: 24rem) {
                html.her #progress-background {
                        display: none;
                }

                html.her #progress-bar {
                        display: none;
                }
        }

        @keyframes pulse {
                50% {
                        opacity: 0.65;
                }
        }
        .animate-pulse-fast {
                animation: pulse 1.5s cubic-bezier(0.4, 0, 0.6, 1) infinite;
        }
</style>
```

