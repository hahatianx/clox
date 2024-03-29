#ifndef CLOX_LINK_LIST_H_
#define CLOX_LINK_LIST_H_

#include "common.h"

typedef struct link {
    struct link *l_prev;
    struct link *l_next;
} list_t, list_link_t;

#define list_link_init(link) \
    do { \
        (link)->l_next = (link)->l_prev = NULL; \
    } while (0)

#define list_init(list) \
    do { \
        (list)->l_next = (list)->l_prev = (list); \
    } while (0)

#define list_insert_after(a, b) \
    do { \
        list_link_t *prev = (a);  \
        list_link_t *next = (b);  \
        next->l_prev = prev; \
        next->l_next = prev->l_next; \
        prev->l_next->l_prev = next; \
        prev->l_next = next; \
    } while (0)

#define list_insert_head(head, link) \
    list_insert_after(head, link)

#define list_insert_tail(head, link) \
    list_insert_after((head)->l_prev, link)

#define list_remove(link) \
    do { \
        list_link_t *cur = (link); \
        list_link_t *prev = cur->l_prev; \
        list_link_t *next = cur->l_next; \
        cur->l_next->l_prev = prev; \
        cur->l_prev->l_next = next; \
        cur->l_prev = cur->l_next = NULL; \
    } while (0)

#define list_remove_head(head) \
    list_remove((head)->l_next)

#define list_remove_tail(head) \
    list_remove((heah)->l_prev)

#define list_object(type, member, link) \
    (type*)((char*)(link) - __offset(type, member));

#define list_head_item(type, member, head) \
    list_object(type, member, (head)->l_next)

#define list_iterate_begin(type, member, head, var) \
    do { \
        list_link_t *_link; \
        list_link_t *_next; \
        for (_link = (head)->l_next; \
             _link != (head); \
             _link = _next) { \
                var = list_object(type, member, _link); \
                _next = _link->l_next;  \
                do

#define list_iterate_end() \
                while (0); \
            } \
    } while (0)

#define list_empty(head) \
    ((head)->l_prev == (head))

#define list_front(head) \
    ((head)->l_next)

//#define list_insert_multi(a, b) \
//    do {                           \
////        list_link_t *head = (a);\
////        list_link_t *l = (b);   \
////        list_link_t *link_f = l->l_next; \
////        list_link_t *link_b = l->l_prev; \
////        l->l_next = l->l_prev = l; \
////        link_b->l_next = head->l_next;   \
////        head->l_next->l_prev = link_b;   \
////        link_f->l_prev = head;     \
////        head->l_next = link_f;     \
//
//    } while (0)

#define list_insert_multi(a, b) \
    list_link_t *head = (a);    \
    list_link_t *l = (b); \
    do { \
        while (!list_empty(l)) { \
            list_link_t *f = list_front(l); \
            list_remove_head(l); \
            list_insert_head(head, f); \
        } \
    } while (0)

#endif