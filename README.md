ngx_http_allow_methods_module
=============================

I couldn't find a way to make Nginx pass custom HTTP verbs to backend applications, so I copy pasted this module together (kudos to the authors of all the other modules).

## Synopsis

This module allows arbitrary HTTP methods to be passed to a backend application.

Example:

    http {
        server {

            location /api/ {
                allow_methods ".*";
            }

        }
    }

## Directives

### allow_methods

###### syntax:
**allow_methods** _pattern_

###### default:
_none_

###### context:
* http
* server
* location
* if in location

###### example:
`allow_methods "^(GET|POST|PUT|DELETE|PATCH|LINK|COPY)$";`

This directive describes HTTP methods that should be passed along. The pattern is case-sensitive (as per [RFC 2616][1]). If it is absent, the default rules of Nginx apply. You should use this directive only in the locations that you really need it in.

  [1]: http://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html#sec5.1.1

## Installation

The installation procedure is the same as for any other module:

    ./configure --add-module=path/to/this/clone

This module has no external dependencies. PCRE support must be enabled.

## Internals

The internal Nginx request struct holds two properties related to the request method. One contains the actual string, e. g. "POST", the other one an id. For custom verbs, this will be set to a `NGX_HTTP_UNKNOWN` and the request will be aborted early with `405 Method not allowed`.

This module tricks all other request handlers into thinking that they are handling a GET request, by setting this id to `NGX_HTTP_GET`, but leaves the string property untouched, so the verb is passed transparently to a backend application.

## Caveats

### Static Resources

This is obviously a hack and there are side effects. For example, if the request is not handled by some application, but instead applies to a static resource, you will always GET that resource, even if the original request was not `GET /resource.txt`. The default behaviour would be to respond with 405.

For fastcgi, you can work around this like so (this is a PHP example, but I guess it works for any fcgi backend):

    location / {
        if (!-f $request_filename) {
            allow_methods ".*";
            rewrite ".*" /app.php;
        }
    }

    location ~ \.php$ {
        fastcgi_pass   unix:/var/run/php-fpm.sock;
        fastcgi_index  app.php;
        fastcgi_param  SCRIPT_FILENAME  $document_root$fastcgi_script_name;
        fastcgi_param  QUERY_STRING     $query_string;
        include        fastcgi_params;
    }

Another option would be to define separate locations:

    location / {
        allow_methods ".*";

        index  app.php;
        try_files $uri $uri/ /app.php?$args;
    }

    location /static {
    }

    location ~ \.php$ {
        fastcgi_pass   unix:/var/run/php-fpm.sock;
        fastcgi_index  app.php;
        fastcgi_param  SCRIPT_FILENAME  $document_root$fastcgi_script_name;
        fastcgi_param  QUERY_STRING     $query_string;
        include        fastcgi_params;
    }


This will GET static resources if they exist, reject non-GET requests to static resources (405) and pass everything else to the application.

### (In-)Compatibility with other modules

Because every request that passes through this request handler internally becomes a GET request, this module may not play nice with other modules (think WebDAV). As long as you keep your locations separated you should be good, but do yourself a favour and test you setup thoroughly before using this module in production. If you ancounter any problems (and hopefully solutions) please drop me a note so I can expand this list.
