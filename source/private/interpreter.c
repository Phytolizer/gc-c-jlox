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

#define NUMBER_DELTA 0.000001

enum interpret_result_type
{
  INTERPRET_RESULT_OK,
  INTERPRET_RESULT_ERROR,
};

struct interpret_result {
  enum interpret_result_type type;
  union {
    struct object ok;
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

static struct runtime_error* runtime_error_new(struct token* token,
                                               const char* message);

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

EXPR_DEFINE_ACCEPT_FOR(struct interpret_result, interpreter);

static struct runtime_error* runtime_error_new(struct token* token,
                                               const char* message)
{
  struct runtime_error* error = GC_MALLOC(sizeof(struct runtime_error));
  error->token = token;
  error->message = message;
  return error;
}

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
  struct object left = left_result.u.ok;
  struct object right = right_result.u.ok;

  struct runtime_error* err;
  switch (expr->op.type) {
    case TOKEN_BANG_EQUAL:
      return INTERPRET_OK(OBJECT_BOOL(!is_equal(&left, &right)));
    case TOKEN_EQUAL_EQUAL:
      return INTERPRET_OK(OBJECT_BOOL(is_equal(&left, &right)));
    case TOKEN_GREATER:
      if ((err = check_number_operands(&expr->op, &left, &right))) {
        return INTERPRET_ERROR(err);
      }
      return INTERPRET_OK(
          OBJECT_BOOL(OBJECT_AS_NUMBER(&left) > OBJECT_AS_NUMBER(&right)));
    case TOKEN_GREATER_EQUAL:
      if ((err = check_number_operands(&expr->op, &left, &right))) {
        return INTERPRET_ERROR(err);
      }
      return INTERPRET_OK(
          OBJECT_BOOL(OBJECT_AS_NUMBER(&left) >= OBJECT_AS_NUMBER(&right)));
    case TOKEN_LESS:
      if ((err = check_number_operands(&expr->op, &left, &right))) {
        return INTERPRET_ERROR(err);
      }
      return INTERPRET_OK(
          OBJECT_BOOL(OBJECT_AS_NUMBER(&left) < OBJECT_AS_NUMBER(&right)));
    case TOKEN_LESS_EQUAL:
      if ((err = check_number_operands(&expr->op, &left, &right))) {
        return INTERPRET_ERROR(err);
      }
      return INTERPRET_OK(
          OBJECT_BOOL(OBJECT_AS_NUMBER(&left) <= OBJECT_AS_NUMBER(&right)));
    case TOKEN_MINUS:
      if ((err = check_number_operands(&expr->op, &left, &right))) {
        return INTERPRET_ERROR(err);
      }
      return INTERPRET_OK(
          OBJECT_NUMBER(OBJECT_AS_NUMBER(&left) - OBJECT_AS_NUMBER(&right)));
    case TOKEN_PLUS:
      if (OBJECT_IS_NUMBER(&left) && OBJECT_IS_NUMBER(&right)) {
        return INTERPRET_OK(
            OBJECT_NUMBER(OBJECT_AS_NUMBER(&left) + OBJECT_AS_NUMBER(&right)));
      }
      if (OBJECT_IS_STRING(&left) && OBJECT_IS_STRING(&right)) {
        return INTERPRET_OK(OBJECT_STRING(alloc_printf(
            "%s%s", OBJECT_AS_STRING(&left), OBJECT_AS_STRING(&right))));
      }
      return INTERPRET_ERROR(runtime_error_new(
          &expr->op, "Operands must be two numbers or two strings."));
    case TOKEN_SLASH:
      if ((err = check_number_operands(&expr->op, &left, &right))) {
        return INTERPRET_ERROR(err);
      }
      return INTERPRET_OK(
          OBJECT_NUMBER(OBJECT_AS_NUMBER(&left) / OBJECT_AS_NUMBER(&right)));
    case TOKEN_STAR:
      if ((err = check_number_operands(&expr->op, &left, &right))) {
        return INTERPRET_ERROR(err);
      }
      return INTERPRET_OK(
          OBJECT_NUMBER(OBJECT_AS_NUMBER(&left) * OBJECT_AS_NUMBER(&right)));
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
  struct object right = right_result.u.ok;

  struct runtime_error* err;
  switch (expr->op.type) {
    case TOKEN_BANG:
      return INTERPRET_OK(OBJECT_BOOL(!is_truthy(&right)));
    case TOKEN_MINUS:
      if ((err = check_number_operand(&expr->op, &right))) {
        return INTERPRET_ERROR(err);
      }
      return INTERPRET_OK(OBJECT_NUMBER(-OBJECT_AS_NUMBER(&right)));
    default:
      ASSERT_UNREACHABLE();
  }
}

void interpret(struct interpreter* interpreter, struct expr* expression)
{
  struct interpret_result result = evaluate(interpreter, expression);
  if (result.type == INTERPRET_RESULT_ERROR) {
    library_runtime_error(result.u.err);
  } else {
    struct object value = result.u.ok;
    printf("%s\n", stringify(&value));
  }
}
