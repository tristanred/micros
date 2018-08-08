#ifndef STATUS_H
#define STATUS_H

enum kernel_error_code
{
    E_OK,
    E_UNKNOWN,
    E_INVALID_PARAM,

    // General error codes

    // General IO Errors
    E_IO_FULL,              // Generated when the target disk cannot field any more requests

    // ATA Driver Errors
};

// If an error enum uses the same convention of 0 == ok then we can use
// these macros for other error types than kernel_error_code.
#define SUCCESS(x) (x == 0)
#define FAILED(x) (x != 0)

#endif