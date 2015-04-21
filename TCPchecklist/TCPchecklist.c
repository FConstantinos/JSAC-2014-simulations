#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<glib.h>
#include"TCPchecklist.h"

typedef struct{
	gboolean *checklist_array;
	long int size;
	long int head;
} TCPchecklist;

static GHashTable *TCPchecklist_hash;
static long int last_key;

static void TCPchecklist_element_destroyer (TCPchecklist *p){

	if(p->size>0){
		free(p->checklist_array);
	}
	free(p); 
}

static void TCPchecklist_key_destroyer (gpointer *p){

}

void TCPchecklist_init(){

	last_key = 0;
	/* Create a hash table containing all TCPchecklists. */
	TCPchecklist_hash = g_hash_table_new_full(&g_direct_hash,&g_direct_equal,(GDestroyNotify)&TCPchecklist_key_destroyer,(GDestroyNotify)&TCPchecklist_element_destroyer);
}

void TCPchecklist_finit(){
	
	/* Destroy hash table and everything in it. */
	g_hash_table_destroy(TCPchecklist_hash); 
}

long int TCPchecklist_create(long int nelem){
	TCPchecklist * list;

	list = (TCPchecklist *)g_malloc(sizeof(TCPchecklist));
	if (nelem>0){
		list->checklist_array = (gboolean *)g_malloc(nelem*sizeof(gboolean));
		memset(list->checklist_array,FALSE,nelem*sizeof(gboolean));
	}
	else {
		list->checklist_array = NULL;
	}

	list->size = nelem;
	list->head = -1;
	last_key++;
	g_hash_table_insert(TCPchecklist_hash,GINT_TO_POINTER(last_key),list);
	return(last_key);
}

void TCPchecklist_destroy(long int id){

	g_hash_table_remove(TCPchecklist_hash,GINT_TO_POINTER(id));
}

long int TCPchecklist_get_largest_consecutive_ACK_id(long int id) {
	TCPchecklist *list;
	gboolean *carray;
	long int size;
	long int head;
	long int i;

	list = (TCPchecklist *)g_hash_table_lookup(TCPchecklist_hash,GINT_TO_POINTER(id));
	carray = list->checklist_array;
	head = list->head;
	size = list->size;

	for (i=head+1;i<size && carray[i]==TRUE;i++){}
	list->head = i-1;
	return(i);
}

void TCPchecklist_add(long int id, long int element){
	TCPchecklist *list;

	list = (TCPchecklist *)g_hash_table_lookup(TCPchecklist_hash,GINT_TO_POINTER(id));
	if (element > 0 && element < list->size ){
		list->checklist_array[element-1] = TRUE;
	}
}
