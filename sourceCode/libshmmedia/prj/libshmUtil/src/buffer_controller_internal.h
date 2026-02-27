/*********************************************************
 *  Copyright 2025 TVU Networks
 *  Licensed under the Apache License, Version 2.0 (the “License”);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *    http://www.apache.org/licenses/LICENSE-2.0
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an “AS IS” BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *********************************************************/
#ifndef BUFFER_CONTROLLER_INTERNAL_H
#define BUFFER_CONTROLLER_INTERNAL_H

#include "buffer_controller.h"
#include <string.h>
#ifdef __APPLE__
#include <stdlib.h>
#else
#include <malloc.h>
#endif
#include <errno.h>

#define BUFF_BITS       10
#define BUFF_MIN_SIZE   (1 << BUFF_BITS)

#define TVU_1LL ((uint64_t)1)

namespace tvushm {

    void BufferCtrlFreeOnlyBuff(BufferController_t *p);
    void BufferCtrlFreeOnlyData(BufferController_t *p);
}

#endif // BUFFER_CONTROLLER_INTERNAL_H
