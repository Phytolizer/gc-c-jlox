#include <assert.h>
#include <gc.h>
#include <lib.h>
#include <math.h>
#include <private/assertions.h>
#include <private/ast/debug.h>
#include <private/interpreter.h>
#include <private/runtime_error.h>
#include <private/strutils.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "private/environment.h"

#define NUMBER_DELTA 0.000001

// #define INTERPRETER_DEBUG

static struct object* lox_clock(struct interpreter* interpreter,
                                struct object_list* parameters)
{
  return OBJECT_NUMBER(time(NULL) - interpreter->init_time);
}

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

enum execution_result_type
{
  EXECUTION_RESULT_TYPE_NONE,
  EXECUTION_RESULT_TYPE_RUNTIME_ERROR,
  EXECUTION_RESULT_TYPE_RETURN,
};

struct execution_result {
  enum execution_result_type type;
  union {
    struct runtime_error* runtime_error;
    struct object* return_value;
  } u;
};

#define EXECUTION_RESULT_NONE \
  (struct execution_result) \
  { \
    .type = EXECUTION_RESULT_TYPE_NONE \
  }
#define EXECUTION_RESULT_RUNTIME_ERROR(err) \
  (struct execution_result) \
  { \
    .type = EXECUTION_RESULT_TYPE_RUNTIME_ERROR, \
    .u = \
    {.runtime_error = (err) } \
  }
#define EXECUTION_RESULT_RETURN(value) \
  (struct execution_result) \
  { \
    .type = EXECUTION_RESULT_TYPE_RETURN, .u = {.return_value = (value) } \
  }

static const char* stringify(struct object* obj);

static bool is_truthy(struct object* obj);
static bool is_equal(struct object* left, struct object* right);
static struct runtime_error* check_number_operand(struct token* op,
                                                  struct object* operand);
static struct runtime_error* check_number_operands(struct token* op,
                                                   struct object* left,
                                                   struct object* right);

#ifdef INTERPRETER_DEBUG
static void interpreter_dump_environment(struct interpreter* interpreter);
#endif

static struct interpret_result interpreter_visit_assign_expr(
    struct interpreter* interpreter, struct assign_expr* expr);
static struct interpret_result interpreter_visit_binary_expr(
    struct interpreter* interpreter, struct binary_expr* expr);
static struct interpret_result interpreter_visit_call_expr(
    struct interpreter* interpreter, struct call_expr* expr);
static struct interpret_result interpreter_visit_grouping_expr(
    struct interpreter* interpreter, struct grouping_expr* expr);
static struct interpret_result interpreter_visit_literal_expr(
    struct interpreter* interpreter, struct literal_expr* expr);
static struct interpret_result interpreter_visit_logical_expr(
    struct interpreter* interpreter, struct logical_expr* expr);
static struct interpret_result interpreter_visit_unary_expr(
    struct interpreter* interpreter, struct unary_expr* expr);
static struct interpret_result interpreter_visit_variable_expr(
    struct interpreter* interpreter, struct variable_expr* expr);

static struct interpret_result interpreter_call(struct interpreter* interpreter,
                                                struct object* callee,
                                                struct object_list* arguments);

static struct execution_result interpreter_visit_block_stmt(
    struct interpreter* interpreter, struct block_stmt* stmt);
static struct execution_result interpreter_visit_expression_stmt(
    struct interpreter* interpreter, struct expression_stmt* stmt);
static struct execution_result interpreter_visit_function_stmt(
    struct interpreter* interpreter, struct function_stmt* stmt);
static struct execution_result interpreter_visit_if_stmt(
    struct interpreter* interpreter, struct if_stmt* stmt);
static struct execution_result interpreter_visit_print_stmt(
    struct interpreter* interpreter, struct print_stmt* stmt);
static struct execution_result interpreter_visit_return_stmt(
    struct interpreter* interpreter, struct return_stmt* stmt);
static struct execution_result interpreter_visit_var_stmt(
    struct interpreter* interpreter, struct var_stmt* stmt);
static struct execution_result interpreter_visit_while_stmt(
    struct interpreter* interpreter, struct while_stmt* stmt);

struct execution_result interpreter_execute(struct interpreter* interpreter,
                                            struct stmt* stmt);
static struct execution_result interpreter_execute_block(
    struct interpreter* interpreter,
    struct stmt_list* statements,
    struct environment* environment);

EXPR_DEFINE_ACCEPT_FOR(struct interpret_result, interpreter)
STMT_DEFINE_ACCEPT_FOR(struct execution_result, interpreter)

static struct interpret_result evaluate(struct interpreter* interpreter,
                                        struct expr* expression)
{
#ifdef INTERPRETER_DEBUG
  printf("[INTP] Evaluating expression: ");
  expr_debug(expression);
  printf("\n");
#endif
  struct interpret_result result =
      expr_accept_interpreter(expression, interpreter);
#ifdef INTERPRETER_DEBUG
  if (result.type == INTERPRET_RESULT_OK) {
    printf("[INTP] ==> ");
    object_print(result.u.ok);
    printf("\n");
  }
#endif
  return result;
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
    case OBJECT_TYPE_NATIVE_FUNCTION:
      return "<native fn>";
  }
  assert(false && "impossible object type");
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
    case OBJECT_TYPE_NATIVE_FUNCTION:
      return OBJECT_IS_NATIVE_FUNCTION(right)
          && OBJECT_AS_NATIVE_FUNCTION(left).func
          == OBJECT_AS_NATIVE_FUNCTION(right).func;
    case OBJECT_TYPE_FUNCTION:
      return OBJECT_IS_FUNCTION(right)
          && OBJECT_AS_FUNCTION(left).declaration
          == OBJECT_AS_FUNCTION(right).declaration;
  }
  assert(false);
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

#ifdef INTERPRETER_DEBUG
static void interpreter_dump_environment(struct interpreter* interpreter)
{
  environment_dump(interpreter->environment);
}
#endif

static struct interpret_result interpreter_visit_assign_expr(
    struct interpreter* interpreter, struct assign_expr* expr)
{
  struct interpret_result value = evaluate(interpreter, expr->value);
  if (value.type == INTERPRET_RESULT_ERROR) {
    return value;
  }
  struct runtime_error* err =
      environment_assign(interpreter->environment, &expr->name, value.u.ok);
  if (err) {
    return INTERPRET_ERROR(err);
  }
  return INTERPRET_OK(value.u.ok);
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

static struct interpret_result interpreter_visit_call_expr(
    struct interpreter* interpreter, struct call_expr* expr)
{
  struct interpret_result callee_result = evaluate(interpreter, expr->callee);
  if (callee_result.type == INTERPRET_RESULT_ERROR) {
    return callee_result;
  }
  struct object* callee = callee_result.u.ok;

  struct object_list* arguments = GC_MALLOC(sizeof(struct object_list));
  LIST_INIT(arguments);
  for (long i = 0; i < expr->arguments->length; ++i) {
    struct interpret_result argument_result =
        evaluate(interpreter, expr->arguments->pointer[i]);
    if (argument_result.type == INTERPRET_RESULT_ERROR) {
      return argument_result;
    }
    LIST_PUSH(arguments, argument_result.u.ok);
  }

  if (!OBJECT_IS_CALLABLE(callee)) {
    return INTERPRET_ERROR(runtime_error_new(
        &expr->paren, "Can only call functions and classes."));
  }
  if (object_arity(callee) != arguments->length) {
    return INTERPRET_ERROR(
        runtime_error_new(&expr->paren,
                          alloc_printf("Expected %li arguments but got %li.",
                                       object_arity(callee),
                                       arguments->length)));
  }
  return interpreter_call(interpreter, callee, arguments);
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

static struct interpret_result interpreter_visit_logical_expr(
    struct interpreter* interpreter, struct logical_expr* expr)
{
  struct interpret_result left = evaluate(interpreter, expr->left);
  if (left.type == INTERPRET_RESULT_ERROR) {
    return left;
  }
  if (expr->op.type == TOKEN_OR) {
    if (is_truthy(left.u.ok)) {
      return left;
    }
  } else {
    if (!is_truthy(left.u.ok)) {
      return left;
    }
  }

  return evaluate(interpreter, expr->right);
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

static struct interpret_result interpreter_call(struct interpreter* interpreter,
                                                struct object* callee,
                                                struct object_list* arguments)
{
  switch (callee->type) {
    case OBJECT_TYPE_NATIVE_FUNCTION:
      return INTERPRET_OK(
          OBJECT_AS_NATIVE_FUNCTION(callee).func(interpreter, arguments));
    case OBJECT_TYPE_FUNCTION: {
      struct function func = OBJECT_AS_FUNCTION(callee);
      struct environment* environment =
          environment_new_enclosed(func.closure);
      for (long i = 0; i < func.declaration->params->length; ++i) {
        environment_define(environment,
                           func.declaration->params->pointer[i].lexeme,
                           // arity was checked, so this is OK
                           arguments->pointer[i]);
      }
      struct execution_result result = interpreter_execute_block(
          interpreter, func.declaration->body, environment);
      switch (result.type) {
        case EXECUTION_RESULT_TYPE_RUNTIME_ERROR:
          return INTERPRET_ERROR(result.u.runtime_error);
        case EXECUTION_RESULT_TYPE_RETURN:
          return INTERPRET_OK(result.u.return_value);
        default:
          return INTERPRET_OK(OBJECT_NULL());
      }
    }
    default:
      ASSERT_UNREACHABLE();
  }
}

static struct execution_result interpreter_visit_block_stmt(
    struct interpreter* interpreter, struct block_stmt* stmt)
{
  return interpreter_execute_block(
      interpreter,
      stmt->statements,
      environment_new_enclosed(interpreter->environment));
}

static struct execution_result interpreter_visit_print_stmt(
    struct interpreter* interpreter, struct print_stmt* stmt)
{
  struct print_stmt* print_stmt = (struct print_stmt*)stmt;
  struct interpret_result result =
      evaluate(interpreter, print_stmt->expression);
  if (result.type == INTERPRET_RESULT_ERROR) {
    return EXECUTION_RESULT_RUNTIME_ERROR(result.u.err);
  }
  printf("%s\n", stringify(result.u.ok));
  return EXECUTION_RESULT_NONE;
}
static struct execution_result interpreter_visit_return_stmt(
    struct interpreter* interpreter, struct return_stmt* stmt)
{
  struct object* value = OBJECT_NULL();
  if (stmt->value) {
    struct interpret_result result = evaluate(interpreter, stmt->value);
    if (result.type == INTERPRET_RESULT_ERROR) {
      return EXECUTION_RESULT_RUNTIME_ERROR(result.u.err);
    }
    value = result.u.ok;
  }
  return EXECUTION_RESULT_RETURN(value);
}
static struct execution_result interpreter_visit_expression_stmt(
    struct interpreter* interpreter, struct expression_stmt* stmt)
{
  struct expression_stmt* expression_stmt = (struct expression_stmt*)stmt;
  struct interpret_result result =
      evaluate(interpreter, expression_stmt->expression);
  if (result.type == INTERPRET_RESULT_ERROR) {
    return EXECUTION_RESULT_RUNTIME_ERROR(result.u.err);
  }
  return EXECUTION_RESULT_NONE;
}

static struct execution_result interpreter_visit_function_stmt(
    struct interpreter* interpreter, struct function_stmt* stmt)
{
  struct function_stmt* function_stmt = (struct function_stmt*)stmt;
  struct object* function = OBJECT_FUNCTION(((struct function) {
      .declaration = function_stmt,
      .closure = interpreter->environment,
  }));
  environment_define(
      interpreter->environment, function_stmt->name.lexeme, function);
  return EXECUTION_RESULT_NONE;
}

static struct execution_result interpreter_visit_if_stmt(
    struct interpreter* interpreter, struct if_stmt* stmt)
{
  struct interpret_result condition = evaluate(interpreter, stmt->condition);
  if (condition.type == INTERPRET_RESULT_ERROR) {
    return EXECUTION_RESULT_RUNTIME_ERROR(condition.u.err);
  }
  if (is_truthy(condition.u.ok)) {
    return interpreter_execute(interpreter, stmt->then_branch);
  }
  if (stmt->else_branch != NULL) {
    return interpreter_execute(interpreter, stmt->else_branch);
  }
  return EXECUTION_RESULT_NONE;
}

static struct execution_result interpreter_visit_var_stmt(
    struct interpreter* interpreter, struct var_stmt* stmt)
{
  struct object* value = OBJECT_NULL();
  if (stmt->initializer) {
    struct interpret_result result = evaluate(interpreter, stmt->initializer);
    switch (result.type) {
      case INTERPRET_RESULT_OK:
        value = result.u.ok;
        break;
      case INTERPRET_RESULT_ERROR:
        return EXECUTION_RESULT_RUNTIME_ERROR(result.u.err);
    }
  }

  environment_define(interpreter->environment, stmt->name.lexeme, value);
  return EXECUTION_RESULT_NONE;
}

static struct execution_result interpreter_visit_while_stmt(
    struct interpreter* interpreter, struct while_stmt* stmt)
{
  while (true) {
    struct interpret_result condition = evaluate(interpreter, stmt->condition);
    if (condition.type == INTERPRET_RESULT_ERROR) {
      return EXECUTION_RESULT_RUNTIME_ERROR(condition.u.err);
    }
    if (!is_truthy(condition.u.ok)) {
      break;
    }
    interpreter_execute(interpreter, stmt->body);
  }
  return EXECUTION_RESULT_NONE;
}

struct interpreter* interpreter_new(void)
{
  struct interpreter* interpreter = GC_MALLOC(sizeof(struct interpreter));
  interpreter->globals = environment_new();
  interpreter->environment = interpreter->globals;
  interpreter->init_time = time(NULL);
  environment_define(
      interpreter->globals, "clock", OBJECT_NATIVE_FUNCTION(0, lox_clock));
  return interpreter;
}

void interpret(struct interpreter* interpreter, struct stmt_list* statements)
{
  for (long i = 0; i < statements->length; ++i) {
    struct execution_result result =
        interpreter_execute(interpreter, statements->pointer[i]);
    if (result.type == EXECUTION_RESULT_TYPE_RUNTIME_ERROR) {
      library_runtime_error(result.u.runtime_error);
      break;
    }
    if (result.type == EXECUTION_RESULT_TYPE_RETURN) {
      ASSERT_UNREACHABLE();
    }
  }
}

struct execution_result interpreter_execute(struct interpreter* interpreter,
                                            struct stmt* stmt)
{
#ifdef INTERPRETER_DEBUG
  printf("[INTP] Executing statement: ");
  stmt_debug(stmt);
  printf("\n");
#endif
  struct execution_result result = stmt_accept_interpreter(stmt, interpreter);
#ifdef INTERPRETER_DEBUG
  if (result.type != EXECUTION_RESULT_TYPE_RUNTIME_ERROR) {
    printf("[INTP] Execution finished successfully\n");
  }
  interpreter_dump_environment(interpreter);
#endif
  return result;
}

static struct execution_result interpreter_execute_block(
    struct interpreter* interpreter,
    struct stmt_list* statements,
    struct environment* environment)
{
  struct environment* previous = interpreter->environment;
  interpreter->environment = environment;

  for (long i = 0; i < statements->length; ++i) {
    struct execution_result result =
        interpreter_execute(interpreter, statements->pointer[i]);
    if (result.type != EXECUTION_RESULT_TYPE_NONE) {
      interpreter->environment = previous;
      return result;
    }
  }

  interpreter->environment = previous;
  return EXECUTION_RESULT_NONE;
}
