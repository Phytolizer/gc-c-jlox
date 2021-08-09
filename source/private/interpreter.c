#include <assert.h>
#include <gc.h>
#include <lib.h>
#include <math.h>
#include <private/assertions.h>
#include <private/interpreter.h>
#include <private/runtime_error.h>
#include <private/strutils.h>
#include <stdbool.h>
#include <string.h>

#include "private/environment.h"

#define NUMBER_DELTA 0.000001

enum interpret_result_type
{
  INTERPRET_RESULT_OK,
  INTERPRET_RESULT_ERROR,
};

struct interpret_result {
  enum interpret_result_type type;
  union {
    struct object* ok;
    struct runtime_error* err;
  } u;
};

#define INTERPRET_OK(value) \
  (struct interpret_result) \
  { \
    .type = INTERPRET_RESULT_OK, .u = {.ok = (value) } \
  }
#define INTERPRET_ERROR(error) \
  (struct interpret_result) \
  { \
    .type = INTERPRET_RESULT_ERROR, .u = {.err = (error) } \
  }

static struct interpret_result evaluate(struct interpreter* interpreter,
                                        struct expr* expression);

static const char* stringify(struct object* obj);

static bool is_truthy(struct object* obj);
static bool is_equal(struct object* left, struct object* right);
static struct runtime_error* check_number_operand(struct token* op,
                                                  struct object* operand);
static struct runtime_error* check_number_operands(struct token* op,
                                                   struct object* left,
                                                   struct object* right);

static struct interpret_result interpreter_visit_binary_expr(
    struct interpreter* interpreter, struct binary_expr* expr);
static struct interpret_result interpreter_visit_grouping_expr(
    struct interpreter* interpreter, struct grouping_expr* expr);
static struct interpret_result interpreter_visit_literal_expr(
    struct interpreter* interpreter, struct literal_expr* expr);
static struct interpret_result interpreter_visit_unary_expr(
    struct interpreter* interpreter, struct unary_expr* expr);
static struct interpret_result interpreter_visit_variable_expr(
    struct interpreter* interpreter, struct variable_expr* expr);

static struct runtime_error* interpreter_visit_print_stmt(
    struct interpreter* interpreter, struct print_stmt* stmt);
static struct runtime_error* interpreter_visit_expression_stmt(
    struct interpreter* interpreter, struct expression_stmt* stmt);
static struct runtime_error* interpreter_visit_var_stmt(
    struct interpreter* interpreter, struct var_stmt* stmt);

struct runtime_error* interpreter_execute(struct interpreter* interpreter,
                                          struct stmt* stmt);

EXPR_DEFINE_ACCEPT_FOR(struct interpret_result, interpreter);
STMT_DEFINE_ACCEPT_FOR(struct runtime_error*, interpreter);

static struct interpret_result evaluate(struct interpreter* interpreter,
                                        struct expr* expression)
{
  return expr_accept_interpreter(expression, interpreter);
}

static const char* stringify(struct object* obj)
{
  switch (obj->type) {
    case OBJECT_TYPE_STRING:
      return OBJECT_AS_STRING(obj);
    case OBJECT_TYPE_NUMBER:
      return alloc_printf("%lg", OBJECT_AS_NUMBER(obj));
    case OBJECT_TYPE_BOOL:
      return alloc_printf("%s", OBJECT_AS_BOOL(obj) ? "true" : "false");
    case OBJECT_TYPE_NULL:
      return "nil";
  }
}

static bool is_truthy(struct object* obj)
{
  if (OBJECT_IS_NULL(obj)) {
    return false;
  }
  if (OBJECT_IS_BOOL(obj)) {
    return OBJECT_AS_BOOL(obj);
  }
  return true;
}

static bool is_equal(struct object* left, struct object* right)
{
  if (OBJECT_IS_NULL(left) && OBJECT_IS_NULL(right)) {
    return true;
  }
  if (OBJECT_IS_NULL(left)) {
    return false;
  }

  switch (left->type) {
    case OBJECT_TYPE_NULL:
      return OBJECT_IS_NULL(right);
    case OBJECT_TYPE_BOOL:
      return OBJECT_IS_BOOL(right)
          && OBJECT_AS_BOOL(left) == OBJECT_AS_BOOL(right);
    case OBJECT_TYPE_STRING:
      return OBJECT_IS_STRING(right)
          && strcmp(OBJECT_AS_STRING(left), OBJECT_AS_STRING(right)) == 0;
    case OBJECT_TYPE_NUMBER:
      return OBJECT_IS_NUMBER(right)
          && fabs(OBJECT_AS_NUMBER(left) - OBJECT_AS_NUMBER(right))
          < NUMBER_DELTA;
  }
}

static struct runtime_error* check_number_operand(struct token* op,
                                                  struct object* operand)
{
  if (OBJECT_IS_NUMBER(operand)) {
    return NULL;
  }
  return runtime_error_new(op, "Operand must be a number.");
}

static struct runtime_error* check_number_operands(struct token* op,
                                                   struct object* left,
                                                   struct object* right)
{
  if (OBJECT_IS_NUMBER(left) && OBJECT_IS_NUMBER(right)) {
    return NULL;
  }
  return runtime_error_new(op, "Operands must be numbers.");
}

static struct interpret_result interpreter_visit_binary_expr(
    struct interpreter* interpreter, struct binary_expr* expr)
{
  struct interpret_result left_result = evaluate(interpreter, expr->left);
  if (left_result.type == INTERPRET_RESULT_ERROR) {
    return left_result;
  }
  struct interpret_result right_result = evaluate(interpreter, expr->right);
  if (right_result.type == INTERPRET_RESULT_ERROR) {
    return right_result;
  }
  struct object* left = left_result.u.ok;
  struct object* right = right_result.u.ok;

  struct runtime_error* err;
  switch (expr->op.type) {
    case TOKEN_BANG_EQUAL:
      return INTERPRET_OK(object_new_bool(!is_equal(left, right)));
    case TOKEN_EQUAL_EQUAL:
      return INTERPRET_OK(object_new_bool(is_equal(left, right)));
    case TOKEN_GREATER:
      if ((err = check_number_operands(&expr->op, left, right))) {
        return INTERPRET_ERROR(err);
      }
      return INTERPRET_OK(
          object_new_bool(OBJECT_AS_NUMBER(left) > OBJECT_AS_NUMBER(right)));
    case TOKEN_GREATER_EQUAL:
      if ((err = check_number_operands(&expr->op, left, right))) {
        return INTERPRET_ERROR(err);
      }
      return INTERPRET_OK(
          object_new_bool(OBJECT_AS_NUMBER(left) >= OBJECT_AS_NUMBER(right)));
    case TOKEN_LESS:
      if ((err = check_number_operands(&expr->op, left, right))) {
        return INTERPRET_ERROR(err);
      }
      return INTERPRET_OK(
          object_new_bool(OBJECT_AS_NUMBER(left) < OBJECT_AS_NUMBER(right)));
    case TOKEN_LESS_EQUAL:
      if ((err = check_number_operands(&expr->op, left, right))) {
        return INTERPRET_ERROR(err);
      }
      return INTERPRET_OK(
          object_new_bool(OBJECT_AS_NUMBER(left) <= OBJECT_AS_NUMBER(right)));
    case TOKEN_MINUS:
      if ((err = check_number_operands(&expr->op, left, right))) {
        return INTERPRET_ERROR(err);
      }
      return INTERPRET_OK(
          object_new_number(OBJECT_AS_NUMBER(left) - OBJECT_AS_NUMBER(right)));
    case TOKEN_PLUS:
      if (OBJECT_IS_NUMBER(left) && OBJECT_IS_NUMBER(right)) {
        return INTERPRET_OK(object_new_number(OBJECT_AS_NUMBER(left)
                                              + OBJECT_AS_NUMBER(right)));
      }
      if (OBJECT_IS_STRING(left) && OBJECT_IS_STRING(right)) {
        return INTERPRET_OK(object_new_string(alloc_printf(
            "%s%s", OBJECT_AS_STRING(left), OBJECT_AS_STRING(right))));
      }
      return INTERPRET_ERROR(runtime_error_new(
          &expr->op, "Operands must be two numbers or two strings."));
    case TOKEN_SLASH:
      if ((err = check_number_operands(&expr->op, left, right))) {
        return INTERPRET_ERROR(err);
      }
      return INTERPRET_OK(
          object_new_number(OBJECT_AS_NUMBER(left) / OBJECT_AS_NUMBER(right)));
    case TOKEN_STAR:
      if ((err = check_number_operands(&expr->op, left, right))) {
        return INTERPRET_ERROR(err);
      }
      return INTERPRET_OK(
          object_new_number(OBJECT_AS_NUMBER(left) * OBJECT_AS_NUMBER(right)));
    default:
      ASSERT_UNREACHABLE();
  }
}

static struct interpret_result interpreter_visit_grouping_expr(
    struct interpreter* interpreter, struct grouping_expr* expr)
{
  return evaluate(interpreter, expr->expression);
}

static struct interpret_result interpreter_visit_literal_expr(
    struct interpreter* interpreter, struct literal_expr* expr)
{
  (void)interpreter;
  return INTERPRET_OK(expr->value);
}

static struct interpret_result interpreter_visit_unary_expr(
    struct interpreter* interpreter, struct unary_expr* expr)
{
  struct interpret_result right_result = evaluate(interpreter, expr->right);
  if (right_result.type == INTERPRET_RESULT_ERROR) {
    return right_result;
  }
  struct object* right = right_result.u.ok;

  struct runtime_error* err;
  switch (expr->op.type) {
    case TOKEN_BANG:
      return INTERPRET_OK(object_new_bool(!is_truthy(right)));
    case TOKEN_MINUS:
      if ((err = check_number_operand(&expr->op, right))) {
        return INTERPRET_ERROR(err);
      }
      return INTERPRET_OK(object_new_number(-OBJECT_AS_NUMBER(right)));
    default:
      ASSERT_UNREACHABLE();
  }
}

static struct interpret_result interpreter_visit_variable_expr(
    struct interpreter* interpreter, struct variable_expr* expr)
{
  struct environment_lookup_result result =
      environment_get(interpreter->environment, &expr->name);
  if (ENVIRONMENT_LOOKUP_RESULT_IS_OK(&result)) {
    return INTERPRET_OK(ENVIRONMENT_LOOKUP_RESULT_GET_OK(&result));
  }
  return INTERPRET_ERROR(ENVIRONMENT_LOOKUP_RESULT_GET_ERROR(&result));
}

static struct runtime_error* interpreter_visit_print_stmt(
    struct interpreter* interpreter, struct print_stmt* stmt)
{
  struct print_stmt* print_stmt = (struct print_stmt*)stmt;
  struct interpret_result result =
      evaluate(interpreter, print_stmt->expression);
  if (result.type == INTERPRET_RESULT_ERROR) {
    return result.u.err;
  }
  printf("%s\n", stringify(result.u.ok));
  return NULL;
}
static struct runtime_error* interpreter_visit_expression_stmt(
    struct interpreter* interpreter, struct expression_stmt* stmt)
{
  struct expression_stmt* expression_stmt = (struct expression_stmt*)stmt;
  struct interpret_result result =
      evaluate(interpreter, expression_stmt->expression);
  if (result.type == INTERPRET_RESULT_ERROR) {
    return result.u.err;
  }
  return NULL;
}

static struct runtime_error* interpreter_visit_var_stmt(
    struct interpreter* interpreter, struct var_stmt* stmt)
{
  struct object* value = NULL;
  if (stmt->initializer) {
    struct interpret_result result = evaluate(interpreter, stmt->initializer);
    switch (result.type) {
      case INTERPRET_RESULT_OK:
        value = result.u.ok;
        break;
      case INTERPRET_RESULT_ERROR:
        return result.u.err;
    }
  }

  environment_define(interpreter->environment, stmt->name.lexeme, value);
  return NULL;
}

struct interpreter* interpreter_new(void)
{
  struct interpreter* interpreter = GC_MALLOC(sizeof(struct interpreter));
  interpreter->environment = environment_new();
  return interpreter;
}

void interpret(struct interpreter* interpreter, struct stmt_list* statements)
{
  for (size_t i = 0; i < statements->length; ++i) {
    struct runtime_error* result =
        interpreter_execute(interpreter, statements->data[i]);
    if (result) {
      library_runtime_error(result);
      break;
    }
  }
}

struct runtime_error* interpreter_execute(struct interpreter* interpreter,
                                          struct stmt* stmt)
{
  return stmt_accept_interpreter(stmt, interpreter);
}
