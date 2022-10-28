#ifndef _internal_h_
#define _internal_h_

#include "enums.h"

#include "desc.h"
#include "errs.h"
#include "env.h"
#include "conn.h"
#include "list.h"
#include "stmt.h"

// make sure `log.h` is included ahead of `taos_helpers.h`, for the `LOG_IMPL` issue
#include "log.h"
#include "taos_helpers.h"

#include <stdatomic.h>

#include <taos.h>

EXTERN_C_BEGIN

typedef struct err_s             err_t;

struct err_s {
  int                         err;
  const char                 *estr;
  SQLCHAR                     sql_state[6];

  char                        buf[1024];

  struct tod_list_head        node;
};

struct errs_s {
  struct tod_list_head        errs;
  struct tod_list_head        frees;
};

#define conn_data_source(_conn) _conn->cfg.dsn ? _conn->cfg.dsn : (_conn->cfg.driver ? _conn->cfg.driver : "")

#define env_append_err(_env, _sql_state, _e, _estr)              \
  errs_append(&_env->errs, "", _sql_state, _e, _estr)

#define conn_append_err(_conn, _sql_state, _e, _estr)                                \
  ({                                                                                 \
    conn_t *__conn = _conn;                                                          \
    errs_append(&__conn->errs, conn_data_source(__conn), _sql_state, _e, _estr);     \
  })

#define stmt_append_err(_stmt, _sql_state, _e, _estr)                                \
  ({                                                                                 \
    stmt_t *__stmt = _stmt;                                                          \
    conn_t *__conn = __stmt->conn;                                                   \
    errs_append(&__stmt->errs, conn_data_source(__conn), _sql_state, _e, _estr);     \
  })

#define env_append_err_format(_env, _sql_state, _e, _fmt, ...)                \
  errs_append_format(&_env->errs, "", _sql_state, _e, _fmt, ##__VA_ARGS__)

#define conn_append_err_format(_conn, _sql_state, _e, _fmt, ...)                                      \
  ({                                                                                                  \
    conn_t *__conn = _conn;                                                                           \
    errs_append_format(&__conn->errs, conn_data_source(__conn), _sql_state, _e, _fmt, ##__VA_ARGS__); \
  })

#define stmt_append_err_format(_stmt, _sql_state, _e, _fmt, ...)                                      \
  ({                                                                                                  \
    stmt_t *__stmt = _stmt;                                                                           \
    conn_t *__conn = __stmt->conn;                                                                    \
    errs_append_format(&__stmt->errs, conn_data_source(__conn), _sql_state, _e, _fmt, ##__VA_ARGS__); \
  })


#define sql_succeeded(_sr) ({ SQLRETURN __sr = _sr; (__sr==SQL_SUCCESS || __sr==SQL_SUCCESS_WITH_INFO); })


struct env_s {
  atomic_int          refc;

  atomic_int          conns;

  errs_t              errs;

  unsigned int        debug:1;
  unsigned int        debug_flex:1;
  unsigned int        debug_bison:1;
};

typedef struct desc_header_s                  desc_header_t;
struct desc_header_s {
  // header fields settable by SQLSetStmtAttr
  SQLULEN             DESC_ARRAY_SIZE;
  SQLUSMALLINT       *DESC_ARRAY_STATUS_PTR;
  SQLULEN            *DESC_BIND_OFFSET_PTR;
  SQLULEN             DESC_BIND_TYPE;
  SQLULEN            *DESC_ROWS_PROCESSED_PTR;

  // header fields else
  SQLUSMALLINT        DESC_COUNT;
};

typedef int (*tsdb_to_sql_c_f)(stmt_t *stmt, const char *data, int len, char *dest, int dlen);

typedef struct desc_record_s                  desc_record_t;

typedef struct sql_c_to_tsdb_meta_s                sql_c_to_tsdb_meta_t;

struct sql_c_to_tsdb_meta_s {
  char          *src_base;
  SQLLEN         src_len;
  desc_record_t *IPD_record;
  TAOS_FIELD_E  *field;
  char          *dst_base;
  int32_t       *dst_len;
};

struct desc_record_s {
  SQLSMALLINT                   DESC_TYPE;
  SQLSMALLINT                   DESC_CONCISE_TYPE;
  SQLULEN                       DESC_LENGTH;
  SQLSMALLINT                   DESC_PRECISION;
  SQLSMALLINT                   DESC_SCALE;
  SQLLEN                        DESC_OCTET_LENGTH;
  SQLPOINTER                    DESC_DATA_PTR;
  SQLLEN                       *DESC_INDICATOR_PTR;
  SQLLEN                       *DESC_OCTET_LENGTH_PTR;
  SQLSMALLINT                   DESC_PARAMETER_TYPE;

  SQLULEN                       element_size_in_column_wise;
  tsdb_to_sql_c_f               conv;

  int                           taos_type;
  int                           taos_bytes;

  void                         *buffer;
  int32_t                      *length;
  char                         *is_null;
  SQLRETURN (*create_buffer_array)(stmt_t *stmt, desc_record_t *record, int rows, TAOS_MULTI_BIND *mb);
  SQLRETURN (*create_length_array)(stmt_t *stmt, desc_record_t *record, int rows, TAOS_MULTI_BIND *mb);

  SQLRETURN (*convf)(stmt_t *stmt, sql_c_to_tsdb_meta_t *meta);
};

struct descriptor_s {
  desc_header_t                 header;

  desc_record_t                *records;
  size_t                        cap;
};

struct desc_s {
  atomic_int                    refc;

  descriptor_t                  descriptor;

  conn_t                       *conn;

  // https://learn.microsoft.com/en-us/sql/odbc/reference/develop-app/types-of-descriptors?view=sql-server-ver16
  // `A row descriptor in one statement can serve as a parameter descriptor in another statement.`
  struct tod_list_head          associated_stmts_as_ARD; // struct stmt_s*
  struct tod_list_head          associated_stmts_as_APD; // struct stmt_s*
};

struct conn_s {
  atomic_int          refc;
  atomic_int          stmts;
  atomic_int          descs;
  atomic_int          outstandings;

  env_t              *env;

  connection_cfg_t    cfg;

  errs_t              errs;

  TAOS               *taos;

  unsigned int        fmt_time:1;
};

typedef struct param_value_s       param_value_t;
struct param_value_s {
  int32_t   length;
  char      is_null;
  union {
    int64_t               tsdb_timestamp;
    int32_t               tsdb_int;
    int64_t               tsdb_bigint;
    const char           *tsdb_varchar;
    const char           *tsdb_nchar;
    void                 *ptr;
  };
  unsigned int            inited:1;
  unsigned int            allocated:1;
};

typedef struct col_s                col_t;
struct col_s {
  TAOS_FIELD          *field;
  int                  i_col;
  const char          *data;
  int                  len;

  SQLSMALLINT          TargetType;

  char                *buf;
  size_t               cap;
  size_t               nr;
};

typedef struct rowset_s             rowset_t;
struct rowset_s {
  int                 i_row;
};

typedef struct stmt_attrs_s         stmt_attrs_t;
struct stmt_attrs_s {
  // SQLULEN                    ATTR_MAX_LENGTH;
};

typedef struct params_s                        params_t;
struct params_s {
  TAOS_MULTI_BIND     *mbs;
  param_value_t       *values;

  size_t               cap;
};

struct stmt_s {
  atomic_int                 refc;

  conn_t                    *conn;

  errs_t                     errs;

  struct tod_list_head       associated_APD_node;
  desc_t                    *associated_APD;

  struct tod_list_head       associated_ARD_node;
  desc_t                    *associated_ARD;

  descriptor_t               APD, IPD;
  descriptor_t               ARD, IRD;

  descriptor_t              *current_APD;
  descriptor_t              *current_ARD;

  stmt_attrs_t               attrs;
  col_t                      current_for_get_data;

  char                      *sql;

  TAOS_STMT                 *stmt;
  // for non-insert-parameterized-statement
  params_t                   params;
  int                        nr_params;

  // for insert-parameterized-statement
  char                      *subtbl;
  TAOS_FIELD_E              *tag_fields;
  int                        nr_tag_fields;
  TAOS_FIELD_E              *col_fields;
  int                        nr_col_fields;

  TAOS_MULTI_BIND           *mbs;
  size_t                     cap_mbs;
  size_t                     nr_mbs;

  TAOS_RES                  *res;
  SQLLEN                     affected_row_count;
  SQLSMALLINT                col_count;
  TAOS_FIELD                *fields;
  int                       *lengths;
  int                        time_precision;

  TAOS_ROW                   rows;
  int                        nr_rows;

  rowset_t                   rowset;

  unsigned int               prepared:1;
  unsigned int               is_insert_stmt:1;
  unsigned int               res_is_from_taos_query:1;
  unsigned int               subtbl_required:1;
};

EXTERN_C_END

#endif // _internal_h_
