
/*
 * Copyright (C) 2013 Peter Schultz <schultz.peter@hotmail.com>
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#define NGX_HTTP_UNKNOWN_BUT_ALLOWED NGX_HTTP_GET

typedef struct {
#if (NGX_PCRE)
    ngx_regex_t  *regex;
#endif
} ngx_http_allow_methods_loc_conf_t;


static ngx_int_t ngx_http_allow_methods_handler(ngx_http_request_t *r);

static void *ngx_http_allow_methods_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_allow_methods_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
static char *ngx_http_allow_methods_set_allow_methods(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_allow_methods_init(ngx_conf_t *cf);


static ngx_command_t  ngx_http_allow_methods_commands[] = {

    { ngx_string("allow_methods"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_1MORE,
      ngx_http_allow_methods_set_allow_methods,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};

static char *
ngx_http_allow_methods_set_allow_methods(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
#if (NGX_PCRE)

    ngx_http_allow_methods_loc_conf_t *clcf = conf;

    ngx_str_t            *value;
    ngx_regex_compile_t   rc;
    u_char                errstr[NGX_MAX_CONF_ERRSTR];

    value = cf->args->elts;

    ngx_memzero(&rc, sizeof(ngx_regex_compile_t));

    rc.pattern = value[1];
    rc.pool = cf->pool;
    rc.options = 0;                    /* case-sensitive */
    rc.err.len = NGX_MAX_CONF_ERRSTR;
    rc.err.data = errstr;

    if (ngx_regex_compile(&rc) != NGX_OK) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "%V", &rc.err);
        return NGX_CONF_ERROR;
    }

    clcf->regex = rc.regex;

    return NGX_CONF_OK;

#else

    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                       "using regex \"%V\" requires PCRE library",
                       regex);
    return NGX_ERROR;

#endif
}

static ngx_http_module_t  ngx_http_allow_methods_module_ctx = {
    NULL,                                           /* preconfiguration */
    ngx_http_allow_methods_init,                     /* postconfiguration */
                                             
    NULL,                                           /* create main configuration */
    NULL,                                           /* init main configuration */
                                             
    NULL,                                           /* create server configuration */
    NULL,                                           /* merge server configuration */

    ngx_http_allow_methods_create_loc_conf,          /* create location configuration */
    ngx_http_allow_methods_merge_loc_conf            /* merge location configuration */
};


ngx_module_t  ngx_http_allow_methods_module = {
    NGX_MODULE_V1,
    &ngx_http_allow_methods_module_ctx,              /* module context */
    ngx_http_allow_methods_commands,                 /* module directives */
    NGX_HTTP_MODULE,                                /* module type */
    NULL,                                           /* init master */
    NULL,                                           /* init module */
    NULL,                                           /* init process */
    NULL,                                           /* init thread */
    NULL,                                           /* exit thread */
    NULL,                                           /* exit process */
    NULL,                                           /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_allow_methods_handler(ngx_http_request_t *r)
{
#if (NGX_PCRE)
    ngx_int_t                         rc;
    ngx_http_allow_methods_loc_conf_t *clcf;
    int                               captures[1];

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http allow_methods handler");

    clcf = ngx_http_get_module_loc_conf(r, ngx_http_allow_methods_module);

    if (clcf->regex == NULL) {
        return NGX_DECLINED;
    }

    rc = ngx_regex_exec(clcf->regex, &r->method_name, captures, 1);

    if (rc == NGX_REGEX_NO_MATCHED) {
        ngx_http_finalize_request(r, NGX_HTTP_NOT_ALLOWED);
    }

    ngx_log_debug2(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http allow_methods faking request method as POST (%d -> %d)", r->method, NGX_HTTP_UNKNOWN_BUT_ALLOWED);

    r->method = NGX_HTTP_UNKNOWN_BUT_ALLOWED;

#endif

    return NGX_DECLINED;
}


static void *
ngx_http_allow_methods_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_allow_methods_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_allow_methods_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

#if 0
    /*
     * set by ngx_pcalloc():
     *
     *     conf->methods = 0;
     */

    conf->min_delete_depth = NGX_CONF_UNSET_UINT;
    conf->access = NGX_CONF_UNSET_UINT;
    conf->create_full_put_path = NGX_CONF_UNSET;
#endif

    return conf;
}


static char *
ngx_http_allow_methods_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_allow_methods_loc_conf_t  *prev = parent;
    ngx_http_allow_methods_loc_conf_t  *conf = child;

#if (NGX_PCRE)
    if (conf->regex == NULL) {
        conf->regex = prev->regex;
    }
#endif

#if 0

    ngx_conf_merge_uint_value(conf->min_delete_depth,
                         prev->min_delete_depth, 0);

    ngx_conf_merge_uint_value(conf->access, prev->access, 0600);

    ngx_conf_merge_value(conf->create_full_put_path,
                         prev->create_full_put_path, 0);
#endif

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_allow_methods_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_allow_methods_handler;

    return NGX_OK;
}
