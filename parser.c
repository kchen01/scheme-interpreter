#include "value.h"
#include "talloc.h"
#include "linkedlist.h"
#include "tokenizer.h"
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef _PARSER
#define _PARSER

// If token is not a close parentheses, push onto stack. 
// Otherwise, pop items from stack until an open paren, forming a subtree. Then push subtree on stack.
Value *addToParseTree(Value  *tree, Value *token, int *depth){
    if(token->type != CLOSE_TYPE){
        tree = cons(token, tree); // push token onto stack
    }
    Value *subtree = makeNull(); // make a new subtree

    switch (token->type) {
        case CLOSE_TYPE:
            (*depth)--; 
            while (car(tree)->type != OPEN_TYPE){
                subtree = cons(car(tree), subtree); // pop and add to subtree
                
                // if there's no more items on the stack, throw an error
                if (cdr(tree)->type == NULL_TYPE){
                    printf("Syntax error: too many close parens\n");
                    texit(0);
                }
                tree = cdr(tree); 
            }
            //subtree->type = CONS_TYPE;
            tree = cdr(tree);
            tree = cons(subtree, tree);
            return tree;

        case OPEN_TYPE:
            (*depth)++;
            return tree;
        default:
            return tree;
    }
}
// Return a pointer to a parse tree representing the structure of a Scheme 
// program, given a list of tokens in the program.
Value *parse(Value *tokens){
    
    assert(tokens != NULL && "Error (parse): null pointer");

    Value *tree = makeNull();
    int depth = 0;

    Value *current = tokens; // linked list of tokens

    while (current->type != NULL_TYPE) {
        Value *token = car(current); // get Value token
        
        tree = addToParseTree(tree, token, &depth); 

        current = cdr(current); // get next node in linked list
    }

    if (depth > 0) {
        printf("Syntax error: too few close parens\n");
        texit(0);
    }
    else if (depth < 0){
        printf("Syntax error: too many close parens\n");
        texit(0);
    }
    return reverse(tree);
}

// Print a parse tree to the screen in a readable fashion. 
void printTree(Value *tree){

    while (tree->type != NULL_TYPE){
        switch(tree->c.car->type){
            case CONS_TYPE: // start of new subtree
                printf("("); 
                printTree(car(tree)); // call recursively
                printf(") ");
                break;
            case INT_TYPE:
                printf("%i ", tree->c.car->i);
                break;
            case DOUBLE_TYPE:
                printf("%f ", tree->c.car->d);
                break;
            case STR_TYPE:
                printf("%s ", tree->c.car->s);
                break;
            case BOOL_TYPE:
                if(tree->c.car->i == 1){
                    printf("#t ");
                } else {
                    printf("#f ");
                }
                break;
            case SYMBOL_TYPE:
                printf("%s ", tree->c.car->s);
                break;
            case NULL_TYPE:
                printf("() ");
                break;
            default:
                ;
        }
        tree = cdr(tree);
    }
    printf("\n");
}

#endif
