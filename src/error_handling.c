#include "error_handling.h"
#include <time.h>

const char *error_code_to_string(error_code_t err_code) {
  // Map error codes to human-readable strings
  switch (err_code) {
    case ERR_SUCCESS:
      return "Success";
    case ERR_NULL_POINTER:
      return "Null pointer error";
    case ERR_INVALID_ARGUMENT:
      return "Invalid argument error";
    case ERR_MEMORY_ALLOCATION:
      return "Memory allocation error";
    case ERR_FILE_NOT_FOUND:
      return "File not found error";
    case ERR_FILE_READ:
      return "File read error";
    case ERR_FILE_WRITE:
      return "File write error";
    case ERR_TIMEOUT:
      return "Timeout error";
    case ERR_BUFFER_OVERFLOW:
      return "Buffer overflow error";
    case ERR_OPERATION_FAILED:
      return "Operation failed error";
    case ERR_PERMISSION_DENIED:
      return "Permission denied error";
    case ERR_RESOURCE_BUSY:
      return "Resource busy error";
    case ERR_INVALID_FORMAT:
      return "Invalid format error";
    case ERR_NOT_FOUND:
      return "Not found error";
    case ERR_INVALID_DATA:
      return "Invalid data error";
    case ERR_INPUT_ERROR:
      return "Input error";
    case ERR_UNKNOWN:
    default:
      return "Unknown error";
  }
}

void print_error(const error_info_t *err_info) {
  if (err_info == NULL) {
    fprintf(stderr, "Error: NULL error info.\n");
    return;
  }

  // Print error code and description
  fprintf(stderr, "ERROR [%d]: %s\n", err_info->code, error_code_to_string(err_info->code));
  
  // Print custom error message
  fprintf(stderr, "Message: %s\n", err_info->message);
  
  // Print location information for debugging
  fprintf(stderr, "Location: %s:%d in %s()\n",
      err_info->file, err_info->line, err_info->function);

  // Print system error if errno is set
  if (errno != 0) {
    fprintf(stderr, "System error: %s\n", strerror(errno));
  }
}

void log_error(const error_info_t *err_info, const char *log_file) {
  if (err_info == NULL || log_file == NULL) {
    fprintf(stderr, "Error: NULL error info or log file.\n");
    return;
  }

  // Open log file in append mode
  FILE *file = fopen(log_file, "a");
  if (file == NULL) {
    fprintf(stderr, "Error opening log file: %s\n", strerror(errno));
    return;
  }

  // Get current timestamp
  time_t now = time(NULL);
  char *time_str = ctime(&now);
  time_str[strlen(time_str) - 1] = '\0'; // Remove trailing newline

  // Write timestamped error information to log file
  fprintf(file, "[%s] ERROR [%d]: %s\n", time_str, err_info->code, error_code_to_string(err_info->code));
  fprintf(file, "Message: %s\n", err_info->message);
  fprintf(file, "Location: %s:%d in %s()\n",
      err_info->file, err_info->line, err_info->function);

  // Include system error information if available
  if (errno != 0) {
    fprintf(file, "System error: %s\n", strerror(errno));
  }

  // Add separator for log entry readability
  fprintf(file, "---\n");
  fclose(file);
}

error_code_t handle_system_error(error_info_t *err_info) {
  if (err_info == NULL) {
    return ERR_NULL_POINTER;
  }

  // Map common errno values to application-specific error codes
  switch (errno) {
    case ENOENT:
      SET_ERROR(err_info, ERR_FILE_NOT_FOUND, strerror(errno));
      return ERR_FILE_NOT_FOUND;
    case EACCES:
      SET_ERROR(err_info, ERR_PERMISSION_DENIED, strerror(errno));
      return ERR_PERMISSION_DENIED;
    case ENOMEM:
      SET_ERROR(err_info, ERR_MEMORY_ALLOCATION, strerror(errno));
      return ERR_MEMORY_ALLOCATION;
    case EBUSY:
      SET_ERROR(err_info, ERR_RESOURCE_BUSY, strerror(errno));
      return ERR_RESOURCE_BUSY;
    case ETIMEDOUT:
      SET_ERROR(err_info, ERR_TIMEOUT, strerror(errno));
      return ERR_TIMEOUT;
    default:
      // Handle unrecognized system errors
      SET_ERROR(err_info, ERR_UNKNOWN, strerror(errno));
      return ERR_UNKNOWN;
  }
}
