/* mbed Microcontroller Library
 * Copyright (c) 2006-2017 ARM Limited
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
#include "mbed.h"
/* EventFlags is to support since mbed-os-5.6. Before then, we need EventFlags_ 
 * to substitute for EventFlags. */
#if (MBED_MAJOR_VERSION <= 5 && MBED_MINOR_VERSION <= 5)

#include "EventFlags_.h"
#include <string.h>
#include "mbed_error.h"
#include "mbed_assert.h"

namespace rtos {

EventFlags_::EventFlags_()
{
    constructor();
}

EventFlags_::EventFlags_(const char *name)
{
    constructor(name);
}

void EventFlags_::constructor(const char *name)
{
    memset(&_obj_mem, 0, sizeof(_obj_mem));
    memset(&_attr, 0, sizeof(_attr));
    _attr.name = name ? name : "application_unnamed_event_flags";
    _attr.cb_mem = &_obj_mem;
    _attr.cb_size = sizeof(_obj_mem);
    _id = osEventFlagsNew(&_attr);
    MBED_ASSERT(_id);
}

uint32_t EventFlags_::set(uint32_t flags)
{
    return osEventFlagsSet(_id, flags);
}

uint32_t EventFlags_::clear(uint32_t flags)
{
    return osEventFlagsClear(_id, flags);
}

uint32_t EventFlags_::get() const
{
    return osEventFlagsGet(_id);
}

uint32_t EventFlags_::wait_all(uint32_t flags, uint32_t timeout, bool clear)
{
    return wait(flags, osFlagsWaitAll, timeout, clear);
}

uint32_t EventFlags_::wait_any(uint32_t flags, uint32_t timeout, bool clear)
{
    return wait(flags, osFlagsWaitAny, timeout, clear);
}

EventFlags_::~EventFlags_()
{
    osEventFlagsDelete(_id);
}

uint32_t EventFlags_::wait(uint32_t flags, uint32_t opt, uint32_t timeout, bool clear)
{
    if (clear == false) {
        opt |= osFlagsNoClear;
    }

    return osEventFlagsWait(_id, flags, opt, timeout);
}

}

#endif  // mbed-os-5.5.6
