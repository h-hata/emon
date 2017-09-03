/*
 * rbtree.h
 *
 *  Created on: 2014/04/02
 *      Author: hata
 */

#ifndef RBTREE_H_
#define RBTREE_H_
typedef void  *RBTree;
RBTree RB_new(void);
int RB_insert(RBTree t, char *key, void *v);
void RB_free(RBTree t);
char *RB_search(RBTree p, char *key);
void RB_print(RBTree p,int t);
#endif /* RBTREE_H_ */
