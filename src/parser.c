#include "parser.h"

struct Parsed_Statement *parse_tokens(const struct Token *tokens, size_t nl);

void parser_free(struct Parsed_Statement *stmt);
