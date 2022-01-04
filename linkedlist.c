#include <stdbool.h>
#include "value.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "talloc.h"

#ifndef _LINKEDLIST
#define _LINKEDLIST

// Create a pointer to a new NULL_TYPE Value (hint: where in memory will 
// the value have to live?
Value *makeNull(){
    Value *nullNode = talloc(sizeof(Value));
    nullNode->type = NULL_TYPE;
    return nullNode;
}

// Return whether the given pointer points at a NULL_TYPE Value. Use assertions 
// to make sure that this is a legitimate operation. See the assignment
// instructions for further explanation on assertions.
bool isNull(Value *value){
    assert(value != NULL && "Error (isNull): value is null");
    if(value->type == NULL_TYPE){
        return true;
    } else {
        return false;
    }
}

// Create a new CONS_TYPE value node.
Value *cons(Value *newCar, Value *newCdr){
    Value *consNode = talloc(sizeof(Value));
    consNode->type = CONS_TYPE;
    consNode->c.car = newCar;
    consNode->c.cdr = newCdr;
    return consNode;
}

// Return a pointer to the car value for the cons cell at the head of the given 
// linked list. Use assertions here to make sure that this is a legitimate operation 
// (e.g., there is no car value at the head of an empty list). See the assignment 
// instructions for further explanation.
Value *car(Value *list){
    assert(list != NULL && "Error (car): car of list is null");
    return list->c.car;
}

// Return a pointer to the cdr value for the cons cell at the head of the given linked
// list. Again use assertions to make sure that this is a legitimate operation.
Value *cdr(Value *list){
    assert(list->c.cdr != NULL && "Error (cdr): cdr of list is null");
    return list->c.cdr;
}


// Display the contents of the linked list to the screen in the
// format of a Scheme list -- e.g., ( 33 "lol" 9.9 ). It's okay
// to just use printf here, though you'll have to add the quotes in
// yourself, for strings.
void display(Value *list){
    while(list->type != NULL_TYPE){
        switch (list->c.car->type) {
            case INT_TYPE:
                printf("%i", list->c.car->i);
                break;
            case DOUBLE_TYPE:
                printf("%g", list->c.car->d);
                break;
            case STR_TYPE:
                printf("%s", list->c.car->s);
                break;
            default:
                printf("");
        }
        list = list->c.cdr;
    }
}

// Return a new list that is the reverse of the one that is passed in. None of
// the values in the original linked list should be copied this time. Instead, 
// create a new linked list of CONS_TYPE nodes whose car values point to the 
// corresponding car values in the original list.
Value *reverse(Value *list){
    Value *reverseHead = makeNull();

    while(list->type != NULL_TYPE){
        Value *newHead = talloc(sizeof(Value));
        newHead = list->c.car;
        newHead->type = list->c.car->type;
        reverseHead = cons(newHead, reverseHead);
        list = list->c.cdr;
    }
    return reverseHead;
}


// Return the length of the given list, i.e., the number of cons cells. 
// Use assertions to make sure that this is a legitimate operation.
int length(Value *value){
    assert(value != NULL && "Error (value): value is null");
    int length = 0;
    while(value->type != NULL_TYPE){
        length++;
        value = value->c.cdr;
    }
    return length;
}


#endif