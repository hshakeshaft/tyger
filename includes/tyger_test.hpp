/**
 * Utility file to add necessary `extern "C" {} ` defs for file testing
*/
#ifndef TYGER_TEST_HPP_
#define TYGER_TEST_HPP_

extern "C" {
  #include "lexer.h"
  #include "lexer_internal.h"
  #include "repl.h"
  #include "tstrings.h"
  #include "parser.h"
  #include "trace.h"
}

#endif // TYGER_TEST_HPP_
