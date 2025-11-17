/******************************************************************************
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
 *********************************************************
 *  Description:
 *      test share memory management {name, key} in Linux Platform
 *  History:
 *      Lotus, Sept 19th 2016
******************************************************************************/
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "sharememory.h"

#define SHM_SUM     100 

pid_t create_shm(const char *name)
{

}

int main(int argc, const char *argv[])
{
    CShareMemory *p = new CShareMemory();
    uint8_t *pshm   = p->CreateOrOpen("dv1", 1024*1024, 10, 5*1024*1024);

    if (!pshm) {
        printf("failed to create dv1\n");
        return -1;
    }
    else {
        printf("create success\n");
    }

    p->CloseMapFile();
    delete p;
    return 0;
}

