#ifndef TYGER_TRACE_H_
#define TYGER_TRACE_H_
#include "parser.h"

typedef enum trace_format
{
  TRACE_YAML,
} Trace_Format;

const char *program_to_string(const Program *p, Trace_Format format);

#endif // TYGER_TRACE_H_
