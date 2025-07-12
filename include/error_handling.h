#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// ==================
// Error Code Enumeration
// ==================

/**
 * Error codes for the application.
 * All error codes are negative values except for success.
 */
typedef enum {
  ERR_SUCCESS = 0,
  ERR_NULL_POINTER = -1,
  ERR_INVALID_ARGUMENT = -2,
  ERR_MEMORY_ALLOCATION = -3,
  ERR_FILE_NOT_FOUND = -4,
  ERR_FILE_READ = -5,
  ERR_FILE_WRITE = -6,
  ERR_TIMEOUT = -7,
  ERR_BUFFER_OVERFLOW = -8,
  ERR_OPERATION_FAILED = -9,
  ERR_PERMISSION_DENIED = -10,
  ERR_RESOURCE_BUSY = -11,
  ERR_INVALID_FORMAT = -12,
  ERR_NOT_FOUND = -13,
  ERR_INVALID_DATA = -14,
  ERR_INPUT_ERROR = -15,
  ERR_UNKNOWN = -99
} error_code_t;

// ==================
// Error Information Structure
// ==================

/**
 * Structure to hold detailed error information including location and context.
 */
typedef struct {
  error_code_t code;
  char message[256];
  const char *file;
  int line;
  const char *function;
} error_info_t;

// ==================
// Error Handling Macros
// ==================

/**
 * Sets error information with code, message, and location details.
 * 
 * @param err_info Pointer to error_info_t structure to populate
 * @param err_code Error code to set
 * @param msg Error message string
 * 
 * @pre err_info must be a valid pointer to error_info_t
 * @post err_info is populated with error details and location information
 * @note This macro automatically captures file, line, and function information
 */
#define SET_ERROR(err_info, err_code, msg) do { \
  (err_info)->code = (err_code); \
  snprintf((err_info)->message, sizeof((err_info)->message), "%s", (msg)); \
  (err_info)->file = __FILE__; \
  (err_info)->line = __LINE__; \
  (err_info)->function = __func__; \
} while (0)

/**
 * Checks if a pointer is NULL and returns with error if true.
 * 
 * @param ptr Pointer to check for NULL
 * @param err_info Error information structure to populate on failure
 * 
 * @pre err_info must be a valid pointer to error_info_t
 * @post On NULL pointer: err_info is populated and function returns ERR_NULL_POINTER
 * @note This macro causes an early return from the calling function on NULL pointer
 */
#define CHECK_NULL(ptr, err_info) do { \
  if ((ptr) == NULL) { \
    SET_ERROR(err_info, ERR_NULL_POINTER, "Null pointer detected"); \
    return ERR_NULL_POINTER; \
  } \
} while (0)

/**
 * Checks if a memory allocation was successful and returns with error if failed.
 * 
 * @param ptr Pointer returned from memory allocation function
 * @param err_info Error information structure to populate on failure
 * 
 * @pre err_info must be a valid pointer to error_info_t
 * @post On allocation failure: err_info is populated and function returns ERR_MEMORY_ALLOCATION
 * @note This macro causes an early return from the calling function on allocation failure
 */
#define CHECK_ALLOCATION(ptr, err_info) do { \
  if ((ptr) == NULL) { \
    SET_ERROR(err_info, ERR_MEMORY_ALLOCATION, "Memory allocation failed"); \
    return ERR_MEMORY_ALLOCATION; \
  } \
} while (0)

// ==================
// Error Handling Function Prototypes
// ==================

/**
 * Converts an error code to its corresponding string representation.
 * 
 * @param err_code Error code to convert
 * @return String representation of the error code
 * 
 * @pre None
 * @post Returns a valid string pointer (never NULL)
 * @note Returns "Unknown error" for unrecognized error codes
 */
const char *error_code_to_string(error_code_t err_code);

/**
 * Prints error information to stderr in a formatted manner.
 * 
 * @param err_info Pointer to error information structure
 * 
 * @pre err_info should be a valid pointer (NULL is handled gracefully)
 * @post Error information is printed to stderr with location details
 * @note Also prints system error information if errno is set
 */
void print_error(const error_info_t *err_info);

/**
 * Logs error information to a specified log file with timestamp.
 * 
 * @param err_info Pointer to error information structure
 * @param log_file Path to the log file to append error information
 * 
 * @pre err_info and log_file should be valid pointers
 * @post Error information is appended to the log file with timestamp
 * @note Creates or appends to the log file; handles file opening errors gracefully
 */
void log_error(const error_info_t *err_info, const char *log_file);

/**
 * Handles system errors by mapping errno values to application error codes.
 * 
 * @param err_info Error information structure to populate
 * @return Application error code corresponding to the current errno value
 * 
 * @pre err_info must be a valid pointer to error_info_t
 * @post err_info is populated with error details based on errno
 * @note Maps common errno values to application-specific error codes
 */
error_code_t handle_system_error(error_info_t *err_info);

#endif // ERROR_HANDLING_H
