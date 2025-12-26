/**
 * @file dso_handle.c
 * @brief Provide __dso_handle for C++ global objects
 * 
 * This symbol is needed for C++ global object destruction
 */

void *__dso_handle __attribute__((weak)) = 0;
