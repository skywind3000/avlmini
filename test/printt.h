//=====================================================================
//
// printt.h - print binary tree to console / file
//
// Created by skywind on 2017/11/10
// Last change: 2017/11/10 22:22:59
//
// usage:
//
// struct MyNode {
//     struct MyNode *left;
//     struct MyNode *right;
//     int key;
// };
//
// void *my_get_child(void *node, int which)
// {
//     struct MyNode *n = (struct MyNode*)node;
//     return (which == 0)? n->left : n->right;
// }
//
// void my_get_text(void *node, char *text)
// {
//     struct MyNode *n = (struct MyNode*)node;
//     sprintf(text, "(%03d)", n->key);
// }
//
// int main(void)
// {
//     struct MyNode *node;
//     ....
//     print_tree_console(node, get_text, get_child);
//     ....
//     return 0;
// }
//
// OUTPUT:
//
//                              (050)
//        +-----------------------+------------------+
//      (020)                                      (080)
//    +---+-------------+                   +--------+--------+
//  (010)             (030)               (060)             (095)
//             +--------+---+           +---+---+         +---+---+
//           (025)        (040)       (055)   (070)     (090)   (098)
//             +---+
//               (028) 
//
//=====================================================================
#ifndef _PRINTT_H__
#define _PRINTT_H__

#include <stddef.h>

#define PRINTT_MAX_STR_SIZE		1024

#define PRINTT_CHILD_LEFT		0
#define PRINTT_CHILD_RIGHT		1

#ifdef __cplusplus
extern "C" {
#endif

void print_tree(void *node,
		void (*get_text)(void *node, char *text),
		void* (*get_child)(void *node, int which),
		void (*output)(const char *line));

void print_tree_console(void *node,
		void (*get_text)(void *node, char *text),
		void* (*get_child)(void *node, int which));

void print_tree_file(void *node,
		void (*get_text)(void *node, char *text),
		void* (*get_child)(void *node, int which),
		const char *filename);

#ifdef __cplusplus
}
#endif


#endif



