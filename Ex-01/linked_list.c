#include <stdlib.h>
#include <stdio.h>
#include "linked_list.h"

list_t list_create()
{
	list_t new_list = malloc(sizeof(list_t));
	new_list->head = NULL;
	new_list->tail = NULL;
	new_list->length = 0;
	return new_list;
}

void list_delete(list_t list)
{
	while(list->head != NULL) {
		list_extract(list, 0);
	}
	free(list);
}

void list_insert(list_t list, int index, int data)
{
	node_t current_node = list->head;
	if (index >= list->length) {
		// We're already at the last element
		list_append(list, data);
		return;
	}
	for (int i = 1; i < index; i++) {
		current_node = current_node -> next;
	}
	node_t new_node = malloc(sizeof(struct node));
	new_node->data = data;
	new_node->next = current_node->next;
	new_node->prev = current_node;
	current_node->next = new_node;
	list->length++;
}

void list_append(list_t list, int data)
{
	node_t new_node = malloc(sizeof(struct node));
	new_node->data = data;
	if (list->head == NULL)
		list->head = new_node;
	new_node->prev = list->tail;
	new_node->next = NULL;
	if (list->tail != NULL) {
		list->tail->next = new_node;
	}
	list->tail = new_node;
	list->length++;
}

void list_print(list_t list)
{
	node_t current_node = list->head;
	while (current_node->next != NULL) {
		printf("%d ", current_node->data);
		current_node = current_node->next;
	}
	printf("%d\n", current_node->data);
}

long list_sum(list_t list)
{
	node_t current_node = list->head;
	long sum = 0;
	while (current_node->next != NULL) {
		sum += current_node->data;
		current_node = current_node->next;
	}
	return (sum + current_node->data);
}

int list_get(list_t list, int index)
{
	if (index >= list->length) {
		printf("Error: requested index out of bounds");
		return -1; // TODO: stupid return value, should assert
	}
	node_t current_node = list->head;
	for (int i = 1; i < index; i++) {
		current_node = current_node -> next;
	}
	return current_node->data;
}

int list_extract(list_t list, int index)
{
	// TODO: Segfault when extracting the last list element
	if (index >= list->length) {
		printf("Error: requested index out of bounds");
		return -1; // TODO: stupid return value, should assert
	}
	node_t current_node = list->head;
	for (int i = 1; i < index; i++) {
		current_node = current_node -> next;
	}
	if (current_node->prev != NULL) {
		current_node->prev->next = current_node->next;
	} else {
		list->head = current_node->next;
	}
	if (current_node->next != NULL) {
		current_node->next->prev = current_node->prev;
	} else {
		list->tail = current_node->prev;
	}
	list->length--;
	int retval = current_node->data;
	free(current_node);
	return retval;
}

