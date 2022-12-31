/*
 * MIT License
 *
 * Copyright (c) 2022 freemine <freemine@yeah.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _log_h_
#define _log_h_

#include "macros.h"

#ifdef _WIN32
void tod_log(const char *fmt, ...) FA_HIDDEN;
// NOTE: make MSVC to do fmt-string-checking with printf during compile-time
#define LOG_IMPL(fmt, ...) (0 ? printf(fmt, ##__VA_ARGS__) : tod_log(fmt, ##__VA_ARGS__))
#else
void tod_log(const char *fmt, ...) __attribute__ ((format (printf, 1, 2))) FA_HIDDEN;
#define LOG_IMPL tod_log
#endif

#include "helpers.h"

#include <stdio.h>
#include <stdlib.h>

EXTERN_C_BEGIN

int tod_get_debug(void) FA_HIDDEN;

#define OD                        D
#define OW                        W
#define OE                        E
#define OA                        A

#define OA_ILE(_statement)        OA(_statement, "internal logic error")
#define OA_NIY(_statement)        OA(_statement, "not implemented yet")
#define OA_DM(_statement)         OA(_statement, "DM logic error")

EXTERN_C_END

#endif // _log_h_

