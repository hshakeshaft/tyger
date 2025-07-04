#pragma once
#include <gtest/gtest.h>
#include "tyger_test.hpp"

/// ScopeExit, adapted from the following Withness blog post to allow for defered blocks
/// http://the-witness.net/news/2012/11/scopeexit-in-c11/
template <typename F>
struct ScopeExit
{
  ScopeExit(F t_f) : f(t_f) {}
  ~ScopeExit() { f(); }
  F f;
};

template <typename F>
ScopeExit<F> MakeScopeExit(F f)
{
  return ScopeExit<F>(f);
}

#define DO_STRING_JOIN(S1, S2) S1 ## S2
#define STRING_JOIN_2(S1, S2) DO_STRING_JOIN(S1, S2)
#define DEFER(...) \
  auto STRING_JOIN_2(defer_scope_exit_, __LINE__) = MakeScopeExit([=] () { __VA_ARGS__ })

///
/// General Utils
///

// #define SETUP_PARSER_TEST_CASE(PARSER, LEXER, INPUT)
#define SETUP_PARSER_TEST_CASE(INPUT)                \
  Lexer lexer; Parser parser; Program p;             \
  do {                                               \
    lexer_init(&lexer, INPUT);                       \
    parser_init(&parser, &lexer);                    \
    p = parser_parse_program(&parser);               \
  } while (0)

#define EXPECT_PROGRAM_PARSED_SUCCESS(P) EXPECT_EQ((P).errors.len, 0)

#define ENUMERATE_PARSER_ERRORS(P)                                      \
  do {                                                                  \
    for (std::size_t i = 0; i < (P).errors.len; ++i) {                  \
      const Tyger_Error *err = &((P).errors.elems[i]);                  \
      EXPECT_FALSE(true) << "[ERROR] " << tyger_error_kind_to_string(err->kind) << '\n'; \
    }                                                                   \
    ASSERT_TRUE((P).errors.len == 0);                                   \
  } while (0)

#define EXPECT_STATEMENT_IS(STMT, KIND)                                 \
  EXPECT_EQ((STMT)->kind, (KIND))                                       \
  << "Expected Statement_Kind " << statement_kind_to_string((KIND))     \
  << ", got " << statement_kind_to_string((KIND))

#define EXPECT_EXPRESSION_IS(EXPR, KIND)                                \
  EXPECT_EQ((EXPR)->kind, (KIND))                                       \
  << "Expected Expression_Kind " << expression_kind_to_string((KIND))   \
  << ", got " << expression_kind_to_string((EXPR)->kind)
