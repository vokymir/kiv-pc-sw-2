#include <string.h>

#include "common.h"
#include "lexer.h"
#include "memory.h"
#include "parser.h"
#include "parser_grammar.h"

struct Parsed_Statement *parse_tokens(const struct Token *tokens[], size_t nl) {
  struct Parsed_Statement *stmt = NULL;
  RETURN_IF_FAIL(tokens, NULL);
  stmt = p_stmt_create(STMT_NONE, nl);
  RETURN_IF_FAIL(stmt, NULL);

  CLEANUP_IF_FAIL(grammar_line(stmt, tokens) == GRM_MATCH);

  return stmt;

cleanup:
  if (stmt) {
    p_stmt_free(&stmt);
  }
  return NULL;
}

struct Parsed_Statement *p_stmt_create(enum Statement_Type stype, size_t nl) {
  struct Parsed_Statement *ps = jalloc(sizeof(struct Parsed_Statement));
  CLEANUP_IF_FAIL(ps);

  CLEANUP_IF_FAIL(p_stmt_init(ps, stype, nl));

  return ps;

cleanup:
  if (ps) {
    jree(ps);
  }
  return NULL;
}

int p_stmt_init(struct Parsed_Statement *ps, enum Statement_Type type,
                size_t nl) {
  CLEANUP_IF_FAIL(ps);

  ps->type = type;
  ps->err = PAR_NO_ERROR;
  ps->line_number = nl;

  switch (ps->type) {
  case STMT_NONE:
  case STMT_KMA:
  case STMT_SECTION_DATA:
  case STMT_SECTION_CODE:
    break;
  case STMT_LABEL_DEF:
    *ps->content.label_def.label_name = 0;
    break;
  case STMT_DATA_DECL:
    memset(&ps->content.data_decl, 0, sizeof(ps->content.data_decl));
    break;
  case STMT_INSTRUCTION:
    memset(&ps->content.instruction, 0, sizeof(ps->content.instruction));
    break;
  case STMT_ERROR:
  default:
    goto cleanup;
  }

  return 1;

cleanup:
  if (ps) {
    p_stmt_deinit(ps);
  }
  return 0;
}

void p_stmt_deinit(struct Parsed_Statement *ps) {
  CLEANUP_IF_FAIL(ps);

  switch (ps->type) {
  case STMT_NONE:
  case STMT_KMA:
  case STMT_SECTION_DATA:
  case STMT_SECTION_CODE:
    break;
  case STMT_LABEL_DEF:
    *ps->content.label_def.label_name = 0;
    break;
  case STMT_DATA_DECL:
    if (ps->content.data_decl.segments) {
      jree(ps->content.data_decl.segments);
    }
    memset(&ps->content.data_decl, 0, sizeof(ps->content.data_decl));
    break;
  case STMT_INSTRUCTION:
    memset(&ps->content.instruction, 0, sizeof(ps->content.instruction));
    break;
  case STMT_ERROR:
  default:
    break;
  }

  ps->type = 0;
  ps->line_number = 0;
  ps->err = 0;

cleanup:
  return;
}

void p_stmt_free(struct Parsed_Statement **stmt) {
  CLEANUP_IF_FAIL(stmt && *stmt);

  p_stmt_deinit(*stmt);
  jree(*stmt);
  *stmt = NULL;

cleanup:
  return;
}
