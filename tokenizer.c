#include <stdbool.h>
#include "value.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "talloc.h"
#include "linkedlist.h"
#include <ctype.h>

#ifndef _TOKENIZER
#define _TOKENIZER


// Returns whether a char is a digit
bool isDigit(char token){
    if(token == '0' || token == '1' 
        || token == '2' || token == '3'
        || token == '4' || token == '5'
        || token == '6' || token == '7'
        || token == '8' || token == '9'){
            return true;
    }
    return false;
}

// Returns whether a char is a letter
bool isLetter(char token){
    if(token == 'a' || token == 'b' || token == 'c' || token == 'd' || 
        token == 'e' || token == 'f' || token == 'g' || token == 'h' || 
        token == 'i' || token == 'j' || token == 'k' || token == 'l' ||
        token == 'm' || token == 'n' || token == 'o' || token == 'p' || 
        token == 'q' || token == 'r' || token == 's' || token == 't' || 
        token == 'u' || token == 'v' || token == 'w' || token == 'x' || 
        token == 'y' || token == 'z' || 
        token == 'A' || token == 'B' || token == 'C' || token == 'D' || 
        token == 'E' || token == 'F' || token == 'G' || token == 'H' || 
        token == 'I' || token == 'J' || token == 'K' || token == 'L' ||
        token == 'M' || token == 'N' || token == 'O' || token == 'P' || 
        token == 'Q' || token == 'R' || token == 'S' || token == 'T' || 
        token == 'U' || token == 'V' || token == 'W' || token == 'X' || 
        token == 'Y' || token == 'Z'){
            return true;
    }
    return false;
}

// Returns whether a token is an integer
bool isInteger(char *token){
    //Allows for first character to be a sign
    if(!(isDigit(token[0]) || token[0] == '-' || token[0] == '+')){
            return false;
    }

    //Makes sure we don't tokenize + or - as an integer
    if((token[0] == '-' || token[0] == '+') && strlen(token) == 1){
        return false;
    }

    int length = strlen(token);
    for(int i = 1; i < length; i++){
        if(!(isDigit(token[i]))){
            return false;
        }
    }
    return true;
}

// Returns whether a token is an double
bool isDouble(char *token){
    bool dotFound = false;

    //Allows for first character to be a sign
    if(!(isDigit(token[0]) || token[0] == '-' || token[0] == '+')){
            return false;
    }

    //Makes sure we don't tokenize + or - as an double
    if((token[0] == '-' || token[0] == '+') && strlen(token) == 1){
        return false;
    }

    int length = strlen(token);
    for(int i = 1; i < length; i++){
        if(!(isDigit(token[i]) || token[i] == '.')){
            return false;
        }
        if(token[i] == '.'){
            if (dotFound == true){
                return false;
            }
            dotFound = true;
        }
    }
    return true;
}

// Returns whether a token is a boolean
bool isBoolean(char *token){
    if(!strcmp(token, "#f") || !strcmp(token, "#t")){
        return true;
    } else {
        return false;
    }
}

// Returns whether a char is the open symbol
bool isOpen(char token){
    if(token == '('){
        return true;
    } else {
        return false;
    }
}

// Returns whether a char is the close symbol
bool isClose(char token){
    if(token == ')'){
        return true;
    } else {
        return false;
    }

}

// Returns whether char is a valid symbol that is not +, -, or .
bool isOtherValidSymbol(char token){
    if(token == '!' || token == '$' || token == '%' || token == '&' || token == '*' || 
    token == '/' || token == ':' || token == '<' || token == '=' || token == '>' || 
    token == '?' || token == '~' || token == '_' || token == '^'){
        return true;
    }
    return false;
}

// Returns whether a char is a valid initial
bool isInitial(char token){
    if(isLetter(token) || isOtherValidSymbol(token)){
        return true;
    } else {
        return false;
    }
}

// Returns whether a char is a valid subsequent
bool isSubsequent(char token){
    if(isInitial(token) || isDigit(token) || token == '.' || token == '+' || token == '-'){
        return true;
    } else {
        return false;
    }
}

// Returns whether a token is a symbol
bool isSymbol(char *token){

    // Tokenizes + or - as a symbol
    if(strlen(token) == 1 && (token[0] == '+' || token[0] == '-')){
        return true;
    }

    // First character can be a letter or valid symbol
    if(!(isInitial(token[0]))){
            return false;
        }

    int length = strlen(token);
    for(int i = 1; i < length; i++){
        if(!(isSubsequent(token[i]))){
            return false;
        }
    }
    return true;
}

//assigns the type and actual value to a Value struct
void assignTypeAndValue(char *tokenString, Value *valToken){
    if(isBoolean(tokenString)){
        if(!strcmp(tokenString, "#t")){
            valToken->i = 1;
        } else {
            valToken->i = 0;
        }
        valToken->type = BOOL_TYPE;
    }
    else if(isInteger(tokenString)){
        char *end = talloc(sizeof(char) * 301);
        valToken->i = strtol(tokenString, &end, 10);
        valToken->type = INT_TYPE;
    }
    else if(isDouble(tokenString)){
        char *end = talloc(sizeof(char) * 301);
        valToken->d = strtod(tokenString, &end);
        valToken->type = DOUBLE_TYPE;
    }
    else if(isSymbol(tokenString)){
        valToken->s = talloc(sizeof(char) * 301);
        strcpy(valToken->s, tokenString);
        valToken->type = SYMBOL_TYPE;
    }
    else {
        printf("Syntax error: cannot tokenize\n");
        texit(0);
    }
}


// Read source code that is input via stdin, and return a linked list consisting of the
// tokens in the source code. Each token is represented as a Value struct instance, where
// the Value's type is set to represent the token type, while the Value's actual value
// matches the type of value, if applicable. For instance, an integer token should have
// a Value struct of type INT_TYPE, with an integer value stored in struct variable i.
// See the assignment instructions for more details. 
Value *tokenize() {

    // Prepare list of tokens
    Value *tokensList = makeNull();

    // Prepare the character stream
    char nextChar;
    nextChar = (char)fgetc(stdin);

    char *current = talloc(sizeof(char) * 301); // current string being built up
    strcpy(current, "");
    for(int i = 0; i < 301; i++){
        current[i] = '\0';
    } // initializes string

    // Start tokenizing!
    while (nextChar != EOF) {

        //if character is the newline character
        if(nextChar == '\n'){
            if(current[0] != '\0'){
                Value *newValCurrent = talloc(sizeof(Value));
                assignTypeAndValue(current, newValCurrent);
                tokensList = cons(newValCurrent, tokensList);

                int length = strlen(current);
                for(int i = 0; i < length; i++){
                    current[i] = '\0';
                } //reset current
            }
        }
        //if character is a comment
        else if(nextChar == ';'){

            if(current[0] != '\0'){
                Value *newValCurrent = talloc(sizeof(Value));
                assignTypeAndValue(current, newValCurrent);
                tokensList = cons(newValCurrent, tokensList);
                int length = strlen(current);

                for(int i = 0; i < length; i++){
                    current[i] = '\0';
                } //reset current
            }

            while(nextChar != '\n'){
                nextChar = (char)fgetc(stdin);
            }

        }
        //if next character is the start of a string
        else if(current[0] == '\0' && nextChar == '\"'){
            current[strlen(current)] = nextChar;
            current[strlen(current)] = '\0';
            nextChar = (char)fgetc(stdin);
            while(nextChar != '\"'){
                current[strlen(current)] = nextChar;
                current[strlen(current)] = '\0';
                nextChar = (char)fgetc(stdin);
                if(nextChar == '\n'){
                    printf("Syntax error: string started but not ended\n");
                    texit(0);
                }
            }
            current[strlen(current)] = '\"';
            current[strlen(current)] = '\0';
            Value *newValString = talloc(sizeof(Value));
            newValString->type = STR_TYPE;
            newValString->s = talloc(sizeof(char) * 301);
            strcpy(newValString->s, current);
            tokensList = cons(newValString, tokensList);

            int length = strlen(current);
            for(int i = 0; i < length; i++){
                current[i] = '\0';
            } //reset current
        }
        // error if current isn't empty

        // if next character is a white space
        else if(nextChar == ' '){
            //assign current string to a token if it isn't emtpy
            if(current[0] != '\0'){
                Value *newValCurrent = talloc(sizeof(Value));
                assignTypeAndValue(current, newValCurrent);
                tokensList = cons(newValCurrent, tokensList);
            }

            int length = strlen(current);
            for(int i = 0; i < length; i++){
                current[i] = '\0';
            } //reset current
        }
        // if next character is a open parentheses
        else if (isOpen(nextChar)){
            //assign current string to a token if it isn't empty
            if(current[0] != '\0'){
                Value *newValCurrent = talloc(sizeof(Value));
                assignTypeAndValue(current, newValCurrent);
                tokensList = cons(newValCurrent, tokensList);
                int length = strlen(current);

                for(int i = 0; i < length; i++){
                    current[i] = '\0';
                } //reset current
            }
            //assign open brace as a token
            Value *newValOpen = talloc(sizeof(Value));
            newValOpen->type = OPEN_TYPE;
            current[0] = nextChar;
            current[1] = '\0';
            newValOpen->s = talloc(sizeof(char) * 301);
            strcpy(newValOpen->s, current);
            tokensList = cons(newValOpen, tokensList);

            int length = strlen(current);
            for(int i = 0; i < length; i++){
                current[i] = '\0';
            } //reset current
        }
        // if next character is a closing parentheses
        else if (isClose(nextChar)){
            //assign current string to a token if it isn't emtpy
            if(current[0] != '\0'){
                Value *newValCurrent = talloc(sizeof(Value));
                assignTypeAndValue(current, newValCurrent);
                tokensList = cons(newValCurrent, tokensList);
                
                int length = strlen(current);
                for(int i = 0; i < length; i++){
                    current[i] = '\0';
                } //reset current
            }
            //assign close brace to a token
            Value *newValClose = talloc(sizeof(Value));
            newValClose->type = CLOSE_TYPE;
            current[0] = ')';
            current[1] = '\0';
            newValClose->s = talloc(sizeof(char) * 301);
            strcpy(newValClose->s, current);
            tokensList = cons(newValClose, tokensList);

            int length = strlen(current);
            for(int i = 0; i < length; i++){
                current[i] = '\0';
            } //reset current
        } else {
            current[strlen(current)] = nextChar;
            current[strlen(current)] = '\0';
        }

        // Read next char
        nextChar = (char)fgetc(stdin);
    }

    // Reverse the tokens list, to put it back in order
    Value *reversedList = reverse(tokensList);
    return reversedList;
}

// Display the contents of the list of tokens, along with associated type information.
// The tokens are displayed one on each line, in the format specified in the instructions.
void displayTokens(Value *list){
    while(list->type != NULL_TYPE){
        switch (list->c.car->type) {
            case INT_TYPE:
                printf("%i:integer\n", list->c.car->i);
                break;
            case DOUBLE_TYPE:
                printf("%f:double\n", list->c.car->d);
                break;
            case STR_TYPE:
                printf("%s:string\n", list->c.car->s);
                break;
            case OPEN_TYPE:
                printf("(:open\n");
                break;
            case CLOSE_TYPE:
                printf("):close\n");
                break;
            case BOOL_TYPE:
                if(list->c.car->i == 1){
                    printf("#t:boolean\n");
                } else {
                    printf("#f:boolean\n");
                }
                break;
            case SYMBOL_TYPE:
                printf("%s:symbol\n", list->c.car->s);
                break;
            default:
                printf("\n");
        }
        list = list->c.cdr;
    }
}

#endif
