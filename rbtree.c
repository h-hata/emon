/*
 ============================================================================
 Name        : rbtree.c
 Author      : Hiroaki Hata
 Version     :
 Copyright   : OLT
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rbtree.h"
typedef enum Color_t { RED, BLACK }Color; // R:赤, B:黒
typedef enum bool_t {FALSE=0,TRUE=1} boolean;
typedef enum branch_t {RIGHT=0,LEFT=1} Branch;
#define KLEN 16 
#define	VLEN 160
typedef struct Node_t {
	Color color;
	char key[KLEN];
	char value[VLEN];
	struct Node_t *l; // 左部分木
	struct Node_t *r; // 右部分木
	struct Node_t *parent;//親
} Node;


static Node *setupNode(Color c, char *k, char *v)
{
	Node *node;
	if(k==NULL || v==NULL){
		return NULL;
	}
	if(strlen(k)>=KLEN || strlen(v)>=VLEN){
		return NULL;
	}
	node=(Node *)malloc(sizeof(Node));
	if(node==NULL){
		return NULL;
	}
	memset(node,0,sizeof(Node));
	node->color=c;
	strncpy(node->key,k,KLEN-1);
	strncpy(node->value,v,VLEN-1);
	return node;
}


// ノード n が赤いかチェックする
static boolean isR(Node *n) { return n != NULL && n->color == RED; }
// ノード n が黒いかチェックする
//static boolean isB(Node *n) { return n != NULL && n->color == BLACK; }

//左回転　新しい根を返す
static Node *rotL(Node *root){
	Node *newroot = root->r;//根の右の子が新しい
	Node *temp = newroot->l;//移動するサブツリー
	newroot->l = root;//立場逆転
	root->r= temp;
	return newroot;
}

//右回転　新しい根を返す
static Node *rotR(Node *root) {
	Node *newroot=root->l;//根の左子が新根
	Node *temp = newroot->r;//移動するサブ木
	newroot->r = root; //root->parent=newroot;
	root->l = temp;// temp->parent=root;
	return newroot;
}

static Node *rotLR(Node *root) {
	Node *newroot;
	newroot= rotL(root->l);
	root->l = newroot;  newroot->parent=root;
	return rotR(root);
}
static Node *rotRL(Node *root) {
	Node *newroot;
	newroot = rotR(root->r);
	root->r = newroot; newroot->parent=root;
	return rotL(root);
}
// エントリー後の並び替え
static Node *balance(Node *root) {
	if (isR(root)) {//ルートが赤なら回転不要
		return root;
	}
	else if (isR(root->l) && isR(root->l->r)) {
		root = rotLR(root);
		root->l->color = BLACK;
	}
	else if (isR(root->l) && isR(root->l->l)) {
		root = rotR(root);
		root->l->color = BLACK;
	}
	else if (isR(root->r) && isR(root->r->l)) {
		root = rotRL(root);
		root->r->color = BLACK;
	}
	else if (isR(root->r) && isR(root->r->r)) {
		root = rotL(root);
		root->r->color = BLACK;
	}
	return root;
}
RBTree RB_new(){
	Node** t;
	t=(Node **)malloc(sizeof(Node *));
	if(t!=NULL){
		*t=NULL;
	}
	return t;
}
/*insert2はループを使う*/
Node *insert2(Node *root, Node *node)
{
	Node *ptr,*par;
	int diff;
	Branch branch;
	for(ptr=root;;){
		diff=strcmp(ptr->key,node->key);
		if (diff > 0) {
			if(ptr->l==NULL){
				ptr->l = node;
				break;
			}else{
				ptr=ptr->l;
			}
		}else if (diff < 0) {
			if(ptr->r==NULL){
				ptr->r = node;
				break;
			}else{
				ptr=ptr->r;
			}
		}else{
			return NULL;//同じキーが発見された
		}
	}
	//ptrは挿入されたノードの親
	node->parent=ptr;
	for(;;){
		par=ptr->parent;
		if(par==NULL){
			break;
		}
		if(par->r==ptr){
			branch=RIGHT;
		}else if (par->l==ptr){
			branch=LEFT;
		}else{
			fprintf(stderr,"Tree collapsed");
			_exit(1);
		}
		ptr=balance(ptr);
		if(branch==RIGHT){
			par->r=ptr;
		}else{
			par->l=ptr;
		}
		ptr=par;
	}
	return ptr;
}



/*insert1は再帰を使う*/
Node *insert1(Node *root, Node *node)
{
	Node *ptr;
	Node *ret;
	ptr=root;
	int diff;

	if (ptr == NULL){
		//最後までたどり着いたら
		node->color=RED;
		return node;//挿入完了(再起せず終わる）
	}
	diff=strcmp(ptr->key,node->key);
	if (diff > 0) {
		ret = insert1(ptr->l, node);
		if(ret==NULL){
			return NULL;
		}else{
			ptr->l=ret;
		}
		return balance(ptr);//ptrは挿入されたノードの
	}else if (diff < 0) {
		ret = insert1(ptr->r, node);
		if(ret==NULL){
			return NULL;
		}else{
			ptr->r=ret;
		}
		return balance(ptr);//ptrは挿入されたノードの親
	}
	return NULL;//同じキーが発見された
}




int RB_insert(RBTree p, char *key, void *v)
{
	Node **t;
	Node *node;
	Node *root;
	if(p==NULL){
		return -10;
	}
	t=p;
	if(p==NULL){
		return -10;
	}
	root=(Node *)*t;
	node=setupNode(RED, key, v);
	if(node==NULL){
		return -20;
	}

	if(root==NULL){
		node->color=BLACK;
		*t=node;
		return 0;
	}
	root = insert1(root, node);//再帰で挿入
	if(root==NULL){
		free(node);
		return -30;
	}
	//常にルートは黒
	root->color=BLACK;
	*t=root;
	return 0;
}

static void print_tree(char *head, char *bar, Node *t) {
	char node[1024];
	char space[1024];

	space[0]='\0';
	if (t != NULL) {
		strcat(space,head);
		strcat(space,"　");
		print_tree(space, "／", t->r);
		strcpy(node,(t->color == RED ? "R" : "B"));
		strcat(node ,":");
		strcat(node ,t->key);
		strcat(node ,":");
		strcat(node ,t->value);
		printf("%s%s%s\n",head,bar,node);
		print_tree(space, "＼", t->l);
	}
}

static void list_tree(Node *t) {
	if (t != NULL) {
		list_tree(t->l);
		printf("%s %s \n",t->key,t->value);
		list_tree(t->r);
	}
}


void RB_print(RBTree p,int type)
{
	Node **t;
	Node *root;
	if(p==NULL){
		return ;
	}
	t=p;
	root=(Node *)*t;
	if(type==0){
		print_tree("", "", root);
	}else if(type==1){
		list_tree(root);
	}
}


static void free_tree(Node *ptr)
{
	Node *c;
	if(ptr==NULL) return;
	c=ptr->l;
	if(c!=NULL){
		if(c->l==NULL && c->r==NULL){
			printf("F:%s %s\n",c->key,c->value);
			free(c);
			ptr->l=NULL;
		}else{
			free_tree(c);
			//再帰から返ってから再度子のチェック
			if(c->l==NULL && c->r==NULL){
				printf("F:%s %s\n",c->key,c->value);
				free(c);
				ptr->l=NULL;
			}
		}
	}
	c=ptr->r;
	if(c!=NULL){
		if(c->r==NULL && c->l==NULL){
			printf("F:%s %s\n",c->key,c->value);
			free(c);
			ptr->r=NULL;
		}else{
			free_tree(c);
			//再帰から返ってから再度子のチェック
			if(c->l==NULL && c->r==NULL){
				printf("F:%s %s\n",c->key,c->value);
				free(c);
				ptr->r=NULL;
			}
		}
	}
}


void RB_free(RBTree p)
{
	Node **t;
	Node *root;
	t=p;
	if(t==NULL){
		return ;
	}
	root=(Node *)*t;
	free_tree(root);
	printf("F:%s %s\n",root->key,root->value);
	free(root);
}



static char *search(Node *ptr, char *k)
{
	int diff;
	if (ptr == NULL){
		return NULL;//挿入完了(再起せず終わる）
	}
	diff=strcmp(ptr->key,k);
	if (diff > 0) {
		return search(ptr->l,k);
	}else if (diff < 0) {
		return search(ptr->r,k);
	}
	return ptr->value;//同じキーが発見された
}


char *RB_search(RBTree p, char *key)
{
	Node **t;
	Node *root;
	t=p;
	if(t==NULL){
		return NULL;
	}
	root=(Node *)*t;
	if(root==NULL){
		return NULL;
	}
	return search(root,key);
}

