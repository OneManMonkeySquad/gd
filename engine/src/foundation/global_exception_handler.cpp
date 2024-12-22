#include "global_exception_handler.h"
#include "print.h"
#include <stacktrace>

namespace fd
{
    LONG WINAPI ExceptionHandler(EXCEPTION_POINTERS* exceptionInfo) {
        DWORD exceptionCode = exceptionInfo->ExceptionRecord->ExceptionCode;
        const void* exceptionAddress = exceptionInfo->ExceptionRecord->ExceptionAddress;

        auto ExceptionCodeToString = [](DWORD code) {
            switch (code) {
            case EXCEPTION_ACCESS_VIOLATION: return "EXCEPTION_ACCESS_VIOLATION";
            case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
            case EXCEPTION_BREAKPOINT: return "EXCEPTION_BREAKPOINT";
            case EXCEPTION_DATATYPE_MISALIGNMENT: return "EXCEPTION_DATATYPE_MISALIGNMENT";
            case EXCEPTION_FLT_DENORMAL_OPERAND: return "EXCEPTION_FLT_DENORMAL_OPERAND";
            case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
            case EXCEPTION_FLT_INEXACT_RESULT: return "EXCEPTION_FLT_INEXACT_RESULT";
            case EXCEPTION_FLT_INVALID_OPERATION: return "EXCEPTION_FLT_INVALID_OPERATION";
            case EXCEPTION_FLT_OVERFLOW: return "EXCEPTION_FLT_OVERFLOW";
            case EXCEPTION_FLT_STACK_CHECK: return "EXCEPTION_FLT_STACK_CHECK";
            case EXCEPTION_FLT_UNDERFLOW: return "EXCEPTION_FLT_UNDERFLOW";
            case EXCEPTION_ILLEGAL_INSTRUCTION: return "EXCEPTION_ILLEGAL_INSTRUCTION";
            case EXCEPTION_IN_PAGE_ERROR: return "EXCEPTION_IN_PAGE_ERROR";
            case EXCEPTION_INT_DIVIDE_BY_ZERO: return "EXCEPTION_INT_DIVIDE_BY_ZERO";
            case EXCEPTION_INT_OVERFLOW: return "EXCEPTION_INT_OVERFLOW";
            case EXCEPTION_INVALID_DISPOSITION: return "EXCEPTION_INVALID_DISPOSITION";
            case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
            case EXCEPTION_PRIV_INSTRUCTION: return "EXCEPTION_PRIV_INSTRUCTION";
            case EXCEPTION_SINGLE_STEP: return "EXCEPTION_SINGLE_STEP";
            case EXCEPTION_STACK_OVERFLOW: return "EXCEPTION_STACK_OVERFLOW";
            default: return "Unknown Exception";
            }
            };

        auto st = std::stacktrace::current();


        fd::println("");
        fd::println("!!!!! CRASH !!!!!");
        fd::println("code={} {}", exceptionCode, ExceptionCodeToString(exceptionCode));

        auto it = std::begin(st);
        std::advance(it, 7); // skip handler
        for (; it != std::end(st); ++it)
        {
            fd::println("{}({}): {}", it->source_file(), it->source_line(), it->description());
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }

    void install_global_exception_handler()
    {
        ::SetUnhandledExceptionFilter(ExceptionHandler);
    }
}