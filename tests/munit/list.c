/*
 * list.c
 *
 * Copyright (c) 2011 - 2023 Marius Zwicker
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>

#include "list.h"

item_t*
createList()
{
    item_t* root = malloc(sizeof(item_t));

    root->data = NULL;
    root->next = NULL;
    return root;
}

int
append(item_t* lst, void* data)
{
    item_t* new_entry;
    item_t* last;

    last = lst;

    while (last->next != NULL) {
        last = last->next;
    }

    new_entry = malloc(sizeof(item_t));

    if (new_entry == NULL) {
        return -1;
    }

    last->next = new_entry;
    new_entry->data = data;
    new_entry->next = NULL;
    return 0;
} /* append */

void
destroyList(item_t* lst) // NOLINT
{
    if (lst->next != NULL) {
        destroyList(lst->next);
    }

    free(lst);
}
