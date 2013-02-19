
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


#if 0
#define NGX_HTTP_DAV_COPY_BLOCK      65536

#define NGX_HTTP_DAV_OFF             2


#define NGX_HTTP_DAV_NO_DEPTH        -3
#define NGX_HTTP_DAV_INVALID_DEPTH   -2
#define NGX_HTTP_DAV_INFINITY_DEPTH  -1
#endif


typedef struct {
#if 0
    ngx_uint_t  methods;
    ngx_uint_t  access;
    ngx_uint_t  min_delete_depth;
    ngx_flag_t  create_full_put_path;
#endif
} ngx_http_allow_method_loc_conf_t;


static ngx_int_t ngx_http_allow_method_handler(ngx_http_request_t *r);

static void *ngx_http_allow_method_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_allow_method_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
static char *ngx_http_allow_method_set_allow_methods(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_allow_method_init(ngx_conf_t *cf);


static ngx_command_t  ngx_http_allow_method_commands[] = {

    { ngx_string("allow_methods"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_1MORE,
      ngx_http_allow_method_set_allow_methods,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};

static char *
ngx_http_allow_method_set_allow_methods(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    //ngx_http_allow_method_loc_conf_t *llcf = conf;

    return NGX_CONF_OK;
}

static ngx_http_module_t  ngx_http_allow_method_module_ctx = {
    NULL,                                           /* preconfiguration */
    ngx_http_allow_method_init,                     /* postconfiguration */
                                             
    NULL,                                           /* create main configuration */
    NULL,                                           /* init main configuration */
                                             
    NULL,                                           /* create server configuration */
    NULL,                                           /* merge server configuration */

    ngx_http_allow_method_create_loc_conf,          /* create location configuration */
    ngx_http_allow_method_merge_loc_conf            /* merge location configuration */
};


ngx_module_t  ngx_http_allow_method_module = {
    NGX_MODULE_V1,
    &ngx_http_allow_method_module_ctx,              /* module context */
    ngx_http_allow_method_commands,                 /* module directives */
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
ngx_http_allow_method_handler(ngx_http_request_t *r)
{
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http allow_method handler");

    r->method = NGX_HTTP_POST;

    return NGX_DECLINED;
}


static void *
ngx_http_allow_method_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_allow_method_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_allow_method_loc_conf_t));
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
ngx_http_allow_method_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
#if 0
    ngx_http_allow_method_loc_conf_t  *prev = parent;
    ngx_http_allow_method_loc_conf_t  *conf = child;

    ngx_conf_merge_bitmask_value(conf->methods, prev->methods,
                         (NGX_CONF_BITMASK_SET|NGX_HTTP_DAV_OFF));

    ngx_conf_merge_uint_value(conf->min_delete_depth,
                         prev->min_delete_depth, 0);

    ngx_conf_merge_uint_value(conf->access, prev->access, 0600);

    ngx_conf_merge_value(conf->create_full_put_path,
                         prev->create_full_put_path, 0);
#endif

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_allow_method_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_allow_method_handler;

    return NGX_OK;
}
