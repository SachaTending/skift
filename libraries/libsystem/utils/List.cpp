
#include <libsystem/core/CString.h>
#include <libsystem/utils/List.h>

List *list_create(void)
{
    List *list = __create(List);

    list->count = 0;
    list->head = nullptr;
    list->tail = nullptr;

    return list;
}

void list_destroy(List *list) { list_destroy_with_callback(list, nullptr); }
void list_destroy_with_callback(List *list, ListDestroyElementCallback callback)
{
    list_clear_with_callback(list, callback);
    free(list);
}

void list_clear(List *list) { list_clear_with_callback(list, nullptr); }
void list_clear_with_callback(List *list, ListDestroyElementCallback callback)
{
    ListItem *current = list->head;

    while (current)
    {
        ListItem *next = current->next;

        if (callback)
        {
            callback(current->value);
        }

        free(current);

        current = next;
    }

    list->count = 0;
    list->head = nullptr;
    list->tail = nullptr;
}

List *list_clone(List *list)
{
    List *copy = list_create();

    list_foreach(void, value, list)
    {
        list_pushback(copy, value);
    }

    return copy;
}

void list_insert_sorted(List *list, void *value, ListCompareElementCallback callback)
{
    if (list->head == nullptr || callback(value, list->head->value))
    {
        list_push(list, value);
    }
    else
    {
        ListItem *current = list->head;

        while (current->next != nullptr && callback(current->next->value, value))
        {
            current = current->next;
        }

        ListItem *item = __create(ListItem);

        item->prev = current;
        item->next = current->next;
        item->value = value;

        if (current->next == nullptr)
        {
            list->tail = item;
        }
        else
        {
            current->next->prev = item;
        }

        current->next = item;

        list->count++;
    }
}

void *list_peek(List *list)
{
    if (list->head != nullptr)
    {
        return list->head->value;
    }
    else
    {
        return nullptr;
    }
}

bool list_peek_and_pushback(List *list, void **value)
{
    *value = list_peek(list);

    if (*value)
    {
        ListItem *item = list->head;

        // Pop
        if (list->count == 1)
        {
            list->count = 0;
            list->head = nullptr;
            list->tail = nullptr;
        }
        else if (list->count > 1)
        {
            item->next->prev = nullptr;
            list->head = item->next;

            list->count--;
        }

        // Push back
        item->prev = nullptr;
        item->next = nullptr;

        list->count++;

        if (list->tail == nullptr)
        {
            list->tail = item;
            list->head = item;
        }
        else
        {
            list->tail->next = item;
            item->prev = list->tail;
            list->tail = item;
        }

        return true;
    }
    else
    {
        return false;
    }
}

void *list_peekback(List *list)
{
    if (list->tail != nullptr)
    {
        return list->tail->value;
    }
    else
    {
        return nullptr;
    }
}

static void list_peekat_from_head(List *list, int index, void **value)
{
    ListItem *current = list->head;

    for (int i = 0; i < index; i++)
    {
        if (current->next)
        {
            current = current->next;
        }
    }

    *value = current->value;
}

static void list_peekat_from_back(List *list, int index, void **value)
{
    ListItem *current = list->tail;

    for (int i = 0; i < (list->count - index - 1); i++)
    {
        if (current->prev)
        {
            current = current->prev;
        }
    }

    *value = current->value;
}

bool list_peekat(List *list, int index, void **value)
{
    *value = nullptr;

    if (list->count >= 1 && index >= 0 && index < list->count)
    {
        if (index < list->count / 2)
        {
            list_peekat_from_head(list, index, value);
        }
        else
        {
            list_peekat_from_back(list, index, value);
        }

        return true;
    }
    else
    {
        return false;
    }
}

int list_indexof(List *list, void *value)
{
    int index = 0;

    list_foreach(void, item, list)
    {
        if (item == value)
        {
            return index;
        }

        index++;
    }

    return -1;
}

void list_insert(List *list, int index, void *value)
{
    if (list_count(list) == 0)
    {
        list_pushback(list, value);
        return;
    }

    if (index == 0)
    {
        list_push(list, value);
    }

    ListItem *current = list->head;

    for (int i = 0; i < index - 1; i++)
    {
        if (current->next)
        {
            current = current->next;
        }
    }

    ListItem *item = __create(ListItem);

    item->prev = current;
    item->next = current->next;
    item->value = value;

    current->next = item;

    if (list->tail == current)
    {
        list->tail = item;
    }

    list->count++;
}

void list_push(List *list, void *value)
{
    ListItem *item = __create(ListItem);

    item->value = value;

    list->count++;

    if (list->head == nullptr)
    {
        list->head = item;
        list->tail = item;
    }
    else
    {
        list->head->prev = item;
        item->next = list->head;
        list->head = item;
    }
}

bool list_pop(List *list, void **value)
{
    ListItem *item = list->head;

    if (list->count == 0)
    {
        return false;
    }
    else if (list->count == 1)
    {
        list->count = 0;
        list->head = nullptr;
        list->tail = nullptr;
    }
    else if (list->count > 1)
    {
        item->next->prev = nullptr;
        list->head = item->next;

        list->count--;
    }

    if (value != nullptr)
    {
        *value = item->value;
    }

    return true;
}

void list_pushback(List *list, void *value)
{
    ListItem *item = __create(ListItem);

    item->prev = nullptr;
    item->next = nullptr;
    item->value = value;

    list->count++;

    if (list->tail == nullptr)
    {
        list->tail = item;
        list->head = item;
    }
    else
    {
        list->tail->next = item;
        item->prev = list->tail;
        list->tail = item;
    }
}

void list_pushback_copy(List *list, void *value, size_t size)
{
    void *copy = malloc(size);
    memcpy(copy, value, size);
    list_pushback(list, copy);
}

bool list_popback(List *list, void **value)
{
    ListItem *item = list->tail;

    if (list->count == 0)
    {
        return false;
    }
    else if (list->count == 1)
    {
        list->count = 0;
        list->head = nullptr;
        list->tail = nullptr;
    }
    else if (list->count > 1)
    {
        item->prev->next = nullptr;
        list->tail = item->prev;

        list->count--;
    }

    if (value != nullptr)
    {
        *value = item->value;
    }

    return true;
}

bool list_remove(List *list, void *value) { return list_remove_with_callback(list, value, nullptr); }
bool list_remove_with_callback(List *list, void *value, ListDestroyElementCallback callback)
{
    for (ListItem *item = list->head; item != nullptr; item = item->next)
    {
        if (item->value == value)
        {
            if (item->prev != nullptr)
            {
                item->prev->next = item->next;
            }
            else
            {
                list->head = item->next;
            }

            if (item->next != nullptr)
            {
                item->next->prev = item->prev;
            }
            else
            {
                list->tail = item->prev;
            }

            list->count--;

            if (callback)
            {
                callback(item->value);
            }

            free(item);

            return true;
        }
    }

    return false;
}

bool list_remove_at(List *list, int index)
{
    return list_remove_at_with_callback(list, index, nullptr);
}

bool list_remove_at_with_callback(List *list, int index, ListDestroyElementCallback callback)
{
    void *value = nullptr;
    list_peekat(list, index, &value);
    return list_remove_with_callback(list, value, callback);
}

bool list_contains(List *list, void *value)
{
    list_foreach(void, item, list)
    {
        if (item == value)
        {
            return true;
        }
    }

    return false;
}

bool list_iterate(List *list, void *target, ListIterationCallback callback)
{
    ListItem *current = list->head;

    while (current)
    {
        ListItem *next = current->next;
        if (callback(target, current->value) == ITERATION_STOP)
        {
            return false;
        }
        current = next;
    }

    return true;
}