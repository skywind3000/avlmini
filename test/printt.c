//=====================================================================
//
// printt.c - print binary tree to console / file
//
// Created by skywind on 2017/11/10
// Last change: 2017/11/10 22:28:12
//
//=====================================================================
#include "printt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


//---------------------------------------------------------------------
// tree driver
//---------------------------------------------------------------------
typedef struct {
	void (*get_text)(void *node, char *text);
	void* (*get_child)(void *node, int which);
	void (*output)(const char *line);
	int paren;
	char text[PRINTT_MAX_STR_SIZE + 8];
	char temp[PRINTT_MAX_STR_SIZE + 8];
}	tree_print_t;



//---------------------------------------------------------------------
// text area
//---------------------------------------------------------------------
typedef struct {
	int w;
	int h;
	int x;
	int y;
	char *ptr;
	char *line[1];
}	text_area_t;


text_area_t *text_area_new(int w, int h)
{
	int stride = (w + 7) & (~7);
	int size = stride * h;
	int require = size + sizeof(text_area_t) + sizeof(char*) * h;
	int j;
	text_area_t *ta = (text_area_t*)malloc(require);
	assert(ta);
	ta->ptr = ((char*)((void*)ta)) + sizeof(text_area_t) + sizeof(char*) * h;
	for (j = 0; j < h; j++) {
		ta->line[j] = (j == 0)? ta->ptr : ta->line[j - 1] + stride;
		memset(ta->line[j], ' ', w);
	}
	ta->w = w;
	ta->h = h;
	ta->x = 0;
	ta->y = 0;
	return ta;
}

void text_area_delete(text_area_t *ta)
{
	assert(ta);
	assert(ta->ptr);
	ta->ptr = NULL;
	ta->w = 0;
	ta->h = 0;
	free(ta);
}

void text_area_putc(text_area_t *ta, int x, int y, char c) 
{
	if (x >= 0 && y >= 0 && x < ta->w && y < ta->h) {
		ta->line[y][x] = c;
	}
}

char text_area_getc(const text_area_t *ta, int x, int y)
{
	if (x >= 0 && y >= 0 && x < ta->w && y < ta->h) {
		return ta->line[y][x];
	}
	return 0;
}

void text_area_copy(text_area_t *dst, int dx, int dy, 
		const text_area_t *src, int x, int y, int w, int h) 
{
	int i, j;
	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			char ch = text_area_getc(src, x + i, y + j);
			text_area_putc(dst, dx + i, dy + j, ch);
		}
	}
}

void text_area_puts(text_area_t *ta, int x, int y, const char *str)
{
	int sx = x;
	for (; str[0]; str++) {
		char ch = str[0];
		if (ch == '\n') x = sx, y++;
		else if (ch == '\r') x = sx;
		else {
			if (ch == '\t') ch = ' ';
			else if (ch < 0x20) ch = '?';
			text_area_putc(ta, x, y, ch);
			x++;
		}
	}
}

void text_area_set_home(text_area_t *ta, int x, int y) 
{
	ta->x = x;
	ta->y = y;
}

void text_area_draw(text_area_t *dst, int x, int y, const text_area_t *src)
{
	if (src == NULL) return;
	text_area_copy(dst, x - src->x, y - src->y, src, 0, 0, src->w, src->h);
}

void text_area_rect(text_area_t *ta, int x, int y, int w, int h, char c)
{
	int i, j;
	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			text_area_putc(ta, x + i, y + j, c);
		}
	}
}

int text_area_padding(const text_area_t *ta, int side)
{
	if (ta == NULL) return 0;
	if (side == 0) return ta->x;
	if (side == 1) return ta->w - ta->x - 1;
	return 0;
}

void text_area_print(const text_area_t *ta)
{
	char *line = (char*)malloc(ta->w + 10);
	int i, j;
	for (j = 0; j < ta->h; j++) {
		memcpy(line, ta->line[j], ta->w);
		line[ta->w] = 0;
		for (i = ta->w; i > 0; i--) {
			if (line[i] != ' ') break;
			line[i] = 0;
		}
		printf("%s\n", line);
	}
	free(line);
}


//---------------------------------------------------------------------
// render
//---------------------------------------------------------------------

text_area_t *tree_print_node(tree_print_t *tp, void *node)
{
	text_area_t *ta = NULL;
	text_area_t *ts = NULL;
	text_area_t *t0 = NULL;
	text_area_t *t1 = NULL;
	char *text = tp->text;
	int size;

	if (node == NULL) {
		return NULL;
	}

	tp->get_text(node, text);
	size = strlen(text);
	ts = text_area_new(size + 2, 1);
	text_area_puts(ts, 1, 0, tp->text);
	ts->x = (ts->w - 0) / 2;
	if (tp->paren) {
		/* text_area_putc(ts, 0, 0, '('); */
		/* text_area_putc(ts, 1 + size, 0, ')'); */
	}

	t0 = tree_print_node(tp, tp->get_child(node, 0));
	t1 = tree_print_node(tp, tp->get_child(node, 1));
	
	if (t0 == NULL && t1 == NULL) {
		return ts;
	}
	else {
		int self_l = text_area_padding(ts, 0);
		int self_r = text_area_padding(ts, 1);
		int c0_l = text_area_padding(t0, 0);
		int c0_r = text_area_padding(t0, 1);
		int c1_l = text_area_padding(t1, 0);
		int c1_r = text_area_padding(t1, 1);
		int padding_l = self_l;
		int padding_r = self_r;
		int child_height = 0;
		int center_s = 0;
		int new_w = 0;
		int new_h = 0;
		int i;
		if (t0) {
			int space = t0->w + 1;
			if (space > padding_l) padding_l = space;
			if (t0->h > child_height) child_height = t0->h;
		}
		if (t1) {
			int space = t1->w + 1;
			if (space > padding_r) padding_r = space;
			if (t1->h > child_height) child_height = t1->h;
		}
		new_w = padding_l + 1 + padding_r;
		new_h = child_height + 2;
		center_s = padding_l;
		ta = text_area_new(new_w, new_h);
		text_area_draw(ta, center_s, 0, ts);
		text_area_delete(ts);
		if (t0) {
			int pos = center_s - (c0_r + 1);
			text_area_draw(ta, pos, 2, t0);
			for (i = pos; i < center_s; i++)
				text_area_putc(ta, i, 1, '-');
			text_area_putc(ta, pos, 1, '+');
			text_area_delete(t0);
		}
		if (t1) {
			int pos = center_s + (c1_l + 1);
			text_area_draw(ta, pos, 2, t1);
			for (i = center_s; i < pos; i++) 
				text_area_putc(ta, i, 1, '-');
			text_area_putc(ta, pos, 1, '+');
			text_area_delete(t1);
		}
		text_area_putc(ta, center_s, 1, '+');
		ta->x = center_s;
		c0_l = c1_r;
	}

	return ta;
}



//---------------------------------------------------------------------
// print_tree 
//---------------------------------------------------------------------
void print_tree(void *node,
		void (*get_text)(void *node, char *text),
		void* (*get_child)(void *node, int which),
		void (*output)(const char *line))
{
	tree_print_t *tp = (tree_print_t*)malloc(sizeof(tree_print_t));
	text_area_t *ta = NULL;
	assert(tp);
	tp->get_text = get_text;
	tp->get_child = get_child;
	tp->output = output;
	tp->paren = 1;
	ta = tree_print_node(tp, node);
	free(tp);
	if (ta) {
		char *line = (char*)malloc(ta->w + 8);
		int i, j;
		for (j = 0; j < ta->h; j++) {
			memcpy(line, ta->line[j], ta->w);
			line[ta->w] = 0;
			for (i = ta->w; i > 0; i--) {
				if (line[i - 1] != ' ') break;
				line[i] = 0;
			}
			if (output) {
				output(line);
			}	else {
				printf("%s\n", line);
			}
		}
		free(line);
		text_area_delete(ta);
	}
}



//---------------------------------------------------------------------
// print to console
//---------------------------------------------------------------------
void print_tree_console(void *node,
		void (*get_text)(void *node, char *text),
		void* (*get_child)(void *node, int which))
{
	print_tree(node, get_text, get_child, NULL);
}

static FILE *_print_tree_fp = NULL;

static void _print_tree_output_file(const char *line)
{
	if (_print_tree_fp) {
		fprintf(_print_tree_fp, "%s\n", line);
	}
}

void print_tree_file(void *node,
		void (*get_text)(void *node, char *text),
		void* (*get_child)(void *node, int which),
		const char *filename)
{
	FILE *fp = fopen(filename, "w");
	if (fp) {
		_print_tree_fp = fp;
		print_tree(node, get_text, get_child, 
				_print_tree_output_file);
		_print_tree_fp = NULL;
		fclose(fp);
	}
}





