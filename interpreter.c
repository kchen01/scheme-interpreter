#include <stdbool.h>
#include "value.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "talloc.h"
#include "linkedlist.h"
#include <ctype.h>
#include "tokenizer.h"
#include "parser.h"
#ifndef _INTERPRETER
#define _INTERPRETER



// Prints the value of one value node
void printValue(Value *value){
    switch (value->type)  {
        case NULL_TYPE: {
            printf("()\n");
            break;
        }
        case CONS_TYPE: {
            printTree(value);
            break;
        }
        case INT_TYPE: {
            printf("%i\n", value->i);
            break;
        }
        case DOUBLE_TYPE: {
            printf("%lf\n", value->d);
            break;
        }
        case BOOL_TYPE: {
            if(value->i == 0){
                printf("#f\n");
            }
            else if(value->i == 1){
                printf("#t\n");
            }
            break;
        }
        case STR_TYPE:{
            printf("%s\n", value->s);
            break;
        }
        case SYMBOL_TYPE: {
            printf("%s\n", value->s);
            break;
        }  
        case VOID_TYPE: {
            printf("");
            break;
        }
        case CLOSURE_TYPE: {
            printf("#<procedure>\n");
            break;
        }
        default:
            printf("none of the types match");
    }
}

//delcare here to use recursively in other functions
Value *eval(Value *tree, Frame *frame);


//checks if a symbol is bound
Value *lookUpSymbol(Value *tree, Frame *frame){
    Value *bindings = frame->bindings;

    while(bindings->type != NULL_TYPE){
        if(!strcmp(car(car(bindings))->s, tree->s)){
            return cdr(car(bindings));
        }
        bindings = cdr(bindings);
    }
    if(frame->parent != NULL){
        return lookUpSymbol(tree, frame->parent);
    } 
    else {
        printf("Evaluation error: unbound variable\n");
        texit(1);
    }
    return tree; // to prevent compiler warning about non-void function
}


//evaluates if statements
Value *evalIf(Value *args, Frame *frame){
    if(car(args)->type != NULL_TYPE){
        Value *evalValue = eval(car(args), frame);
        // checks boolean values
        if (evalValue->type == BOOL_TYPE){
            int evalBool = evalValue -> i; // get whether it's true or false
            if (evalBool == 1){
                if(cdr(args)->type != NULL_TYPE){
                    return eval(car(cdr(args)), frame);
                } 
                else {
                    printf("Evaluation error: not enough arguments to if statement\n");
                    texit(1);
                }
            }
            else if (evalBool == 0){
                if(cdr(cdr(args))->type != NULL_TYPE){
                    return eval(car(cdr(cdr(args))), frame);
                } 
                else {
                    printf("Evaluation error: not enough arguments to if statement\n");
                    texit(1);
                }
            }
        } 
        else {
            return eval(car(cdr(args)), frame); // not BOOL_TYPE, so treat as truthy
        }
    }
    else {
        printf("Evaluation error: not enough arguments to if statement\n");
        texit(1);
    }
    return args; // to prevent compiler warning about non-void function
}

//evaluates let statements
Value *evalLet(Value *args, Frame *frame){
    Frame *f = talloc(sizeof(Frame));
    f -> parent = frame;
    f -> bindings = makeNull();

    Value *list = car(args);
    Value *body = cdr(args);

    //if a list is null but there is a body return the last element
    if(list->type == NULL_TYPE && body->type != NULL_TYPE){
        while(cdr(body)->type != NULL_TYPE){
            eval(car(body), f);
            body = cdr(body);
        }
        return eval(car(body), f);
    }

    //if the list isn't a list of lists or null throw an error
    if(list->type != CONS_TYPE && list->type != NULL_TYPE){
        printf("Evaluation error: bad form in let\n");
        texit(1);  
    }

    //if there is a null binding throw an error
    if((list->type == CONS_TYPE) && (car(list)->type == NULL_TYPE)){
        printf("Evaluation error: null binding in let\n");
        texit(1);
    }

    //if there is no body throw an error
    if(body->type == NULL_TYPE){
        printf("Evaluation error: no args following the bindings in let\n");
        texit(1);
    }

    //if there is not a list of lists throw an error
    if(car(list)->type != CONS_TYPE){
        printf("Evaluation error: bad form in let\n");
        texit(1); 
    }


    //iterate through the list of lists
    while (list->type != NULL_TYPE){
        Value *sublist = car(list);

        Value *var_i = car(sublist);

        // if the first thing isn't a symbol throw an error
        if(var_i->type != SYMBOL_TYPE){
            printf("Evaluation error: first argument of each sublist must be a symbol\n");
            texit(1);
        }

        Value *val_i = eval(car(cdr(sublist)), frame);

        Value *bindings = f->bindings;
        //iterate through bindings to find potential duplicates
        while(bindings->type != NULL_TYPE){
            if(!strcmp(car(car(bindings))->s, var_i->s)){
                printf("Evaluation error: duplicate variable in let\n");
                texit(1);
            }
            bindings = cdr(bindings);
        }
        f->bindings = cons(cons(var_i, val_i), f->bindings);
        list = cdr(list);
    }

    while(cdr(body)->type != NULL_TYPE){
        eval(car(body), f);
        body = cdr(body);
    }
    return eval(car(body), f);

}

//evaluates quote statements
Value *evalQuote(Value *args){
    if(args->type == NULL_TYPE){
        printf("Evaluation error: no arguments to quote\n");
        texit(1);
    }
    if(cdr(args)->type != NULL_TYPE){
        printf("Evaluation error: multiple arguments to quote\n");
        texit(1);
    }
    return args;
}



//evaluates let* statements
Value *evalLetStar(Value *args, Frame *frame){

    Value *list = car(args);
    Value *body = cdr(args);

    //if a list is null but there is a body return the last element
    if(list->type == NULL_TYPE && body->type != NULL_TYPE){
        while(cdr(body)->type != NULL_TYPE){
            eval(car(body), frame);
            body = cdr(body);
        }
        return eval(car(body), frame);
    }

    //if the list isn't a list of lists or null throw an error
    if(list->type != CONS_TYPE && list->type != NULL_TYPE){
        printf("Evaluation error: bad form in let\n");
        texit(1);  
    }

    //if there is a null binding throw an error
    if((list->type == CONS_TYPE) && (car(list)->type == NULL_TYPE)){
        printf("Evaluation error: null binding in let\n");
        texit(1);
    }

    //if there is no body throw an error
    if(body->type == NULL_TYPE){
        printf("Evaluation error: no args following the bindings in let\n");
        texit(1);
    }

    //if there is not a list of lists throw an error
    if(car(list)->type != CONS_TYPE){
        printf("Evaluation error: bad form in let\n");
        texit(1); 
    }

    Frame *parent = frame;
    Frame *f;

    //iterate through the list of lists
    while (list->type != NULL_TYPE){
        f = talloc(sizeof(Frame));
        f -> parent = parent;
        f -> bindings = makeNull();

        Value *sublist = car(list);

        Value *var_i = car(sublist);

        // if the first thing isn't a symbol throw an error
        if(var_i->type != SYMBOL_TYPE){
            printf("Evaluation error: first argument of each sublist must be a symbol\n");
            texit(1);
        }


        Value *val_i = eval(car(cdr(sublist)), parent);

        Value *bindings = f->bindings;
        f->bindings = cons(cons(var_i, val_i), f->bindings);
        list = cdr(list);
        parent = f;
    }

    while(cdr(body)->type != NULL_TYPE){
        eval(car(body), f);
        body = cdr(body);
    }
    return eval(car(body), f);

    return args; // to prevent compiler warning about non-void function
}


//evaluates letrec statements
Value *evalLetRec(Value *args, Frame *frame){
    Frame *env2 = talloc(sizeof(Frame));
    env2 -> parent = frame;
    env2 -> bindings = makeNull();


    Value *unspecifiedList = car(args);
    Value *unspecifiedVal = talloc(sizeof(Value));
    unspecifiedVal->type = UNSPECIFIED_TYPE;
    while(unspecifiedList->type != NULL_TYPE){
        Value *unspecifiedSublist = car(unspecifiedList);
        Value *unspecifiedVar_i = car(unspecifiedSublist);

        env2->bindings = cons(cons(unspecifiedVar_i, unspecifiedVal), env2->bindings);
        unspecifiedList = cdr(unspecifiedList);
    }

    unspecifiedList = car(args);

    while(unspecifiedList->type != NULL_TYPE){
        if(eval(car(cdr(car(unspecifiedList))), env2)->type == UNSPECIFIED_TYPE){
            printf("Evaluation error: bindings not created yet\n");
            texit(1);
        }
        unspecifiedList = cdr(unspecifiedList);
    }

    env2->bindings = makeNull();

    Value *list = car(args);
    Value *body = cdr(args);

    //if a list is null but there is a body return the last element
    if(list->type == NULL_TYPE && body->type != NULL_TYPE){
        while(cdr(body)->type != NULL_TYPE){
            eval(car(body), env2);
            body = cdr(body);
        }
        return eval(car(body), env2);
    }

    //if the list isn't a list of lists or null throw an error
    if(list->type != CONS_TYPE && list->type != NULL_TYPE){
        printf("Evaluation error: bad form in let\n");
        texit(1);  
    }

    //if there is a null binding throw an error
    if((list->type == CONS_TYPE) && (car(list)->type == NULL_TYPE)){
        printf("Evaluation error: null binding in let\n");
        texit(1);
    }

    //if there is no body throw an error
    if(body->type == NULL_TYPE){
        printf("Evaluation error: no args following the bindings in let\n");
        texit(1);
    }

    //if there is not a list of lists throw an error
    if(car(list)->type != CONS_TYPE){
        printf("Evaluation error: bad form in let\n");
        texit(1); 
    }


    //iterate through the list of lists
    while (list->type != NULL_TYPE){
        Value *sublist = car(list);

        Value *var_i = car(sublist);

        // if the first thing isn't a symbol throw an error
        if(var_i->type != SYMBOL_TYPE){
            printf("Evaluation error: first argument of each sublist must be a symbol\n");
            texit(1);
        }

        Value *val_i = eval(car(cdr(sublist)), env2);

        Value *bindings = env2->bindings;
        //iterate through bindings to find potential duplicates
        while(bindings->type != NULL_TYPE){
            if(!strcmp(car(car(bindings))->s, var_i->s)){
                printf("Evaluation error: duplicate variable in let\n");
                texit(1);
            }
            bindings = cdr(bindings);
        }
        env2->bindings = cons(cons(var_i, val_i), env2->bindings);
        list = cdr(list);
    }

    while(cdr(body)->type != NULL_TYPE){
        eval(car(body), env2);
        body = cdr(body);
    }
    return eval(car(body), env2);

}


// evaluates cond
Value *evalCond(Value *args, Frame *frame){
    while(args->type != NULL_TYPE){
        if(car(car(args))->type == SYMBOL_TYPE){
            if(!strcmp(car(car(args))->s, "else")){
                return eval(car(cdr(car(args))), frame);
            } else {
                printf("Evaluation error: unrecognized symbol in cond\n");
                texit(1);
            }
        }

        Value *evalCarCar = eval(car(car(args)), frame);
        if(evalCarCar->type == BOOL_TYPE){
            if(evalCarCar->i == 1){
                return eval(car(cdr(car(args))), frame);
            }
        }
        
        args = cdr(args);
    }
    Value *nothingTrue = talloc(sizeof(Value));
    nothingTrue->type = VOID_TYPE;
    return nothingTrue;
}


//evaluates expressions
Value *evalDefine(Value *args, Frame *frame){
    if(args->type == NULL_TYPE){
        printf("Evaluation error: no args following define\n");
        texit(1);
    }
    if(cdr(args)->type == NULL_TYPE){
        printf("Evaluation error: no value following the symbol in define\n");
        texit(1);
    }
    if(car(args)->type != SYMBOL_TYPE){
        printf("Evaluation error: define must bind to a symbol\n");
        texit(1);
    }
    frame->bindings = cons(cons(car(args), eval(car(cdr(args)), frame)), frame->bindings);
    Value *voidNode = makeNull();
    voidNode->type = VOID_TYPE;
    return voidNode;
}

//evaluates set expressions
Value *evalSet(Value *args, Frame *frame, Frame *originalFrame){
    if(args->type == NULL_TYPE){
        printf("Evaluation error: no args following set!\n");
        texit(1);
    }
    if(cdr(args)->type == NULL_TYPE){
        printf("Evaluation error: no value following the symbol in define\n");
        texit(1);
    }
    if(car(args)->type != SYMBOL_TYPE){
        printf("Evaluation error: define must bind to a symbol\n");
        texit(1);
    }
    bool varFound = false;
    Value *bindings = frame->bindings;
    Value *newBindings = makeNull();
    Value *newVal = eval(car(cdr(args)), originalFrame);
    while(bindings->type != NULL_TYPE){
        if(!strcmp(car(args)->s, car(car(bindings))->s)){
            newBindings = cons(cons(car(args), newVal), newBindings);
            varFound = true;
        } else {
            newBindings = cons(cons(car(car(bindings)), cdr(car(bindings))), newBindings);
        }
        bindings = cdr(bindings);
    }
    if(varFound){
        frame->bindings = newBindings;
    } else {
        if(frame->parent == NULL){
            printf("Evaluation error: variable not defined before set! statement\n");
            texit(1);
        } else {
            return evalSet(args, frame->parent, originalFrame);
        }
    }
    Value *voidNode = makeNull();
    voidNode->type = VOID_TYPE;
    return voidNode;
}

//checks if a list of symbols contains a target symbol
bool contains(Value *list, Value *target){
    while(list->type != NULL_TYPE){
        if(!strcmp(target->s, car(list)->s)){
            return true;
        }
        list = cdr(list);
    }
    return false;
}

//evaluate define expressions
Value *evalLambda(Value *args, Frame *frame){
    if(args->type == NULL_TYPE){
        printf("Evaluation error: no args following lambda\n");
        texit(1);
    }
    Value *closureValue = talloc(sizeof(Value));
    closureValue->type = CLOSURE_TYPE;

    Value *nonDuplicateParams = makeNull();

    Value *targets = car(args);
    while(targets->type != NULL_TYPE){
        if(car(targets)->type != SYMBOL_TYPE){
            printf("Evaluation error: formal parameters for lambda must be symbols\n");
            texit(1);
        }
        if(contains(nonDuplicateParams, car(targets))){
            printf("Evaluation error: duplicate identifier in lambda\n");
            texit(1);
        }
        nonDuplicateParams = cons(car(targets), nonDuplicateParams);
        targets = cdr(targets);
    }

    closureValue->cl.paramNames = car(args);

    if(cdr(args)->type == NULL_TYPE){
        printf("Evaluation error: no function code\n");
        texit(1);
    }
    closureValue->cl.functionCode = car(cdr(args));

    closureValue->cl.frame = frame;
    return closureValue;
}

// evaluates and statements
Value *evalAnd(Value *args, Frame *frame){
    Value *andReturn = talloc(sizeof(Value));
    andReturn->type = BOOL_TYPE;
    while(args->type != NULL_TYPE){
        Value *evalCar = eval(car(args), frame);
        if(evalCar->type == BOOL_TYPE){
            if(evalCar->i == 0){
                andReturn->i = 0;
                return andReturn;
            }
        }
        args = cdr(args);
    }
    andReturn->i = 1;
    return andReturn;
}

// evaluates or statements
Value *evalOr(Value *args, Frame *frame){
    Value *orReturn = talloc(sizeof(Value));
    orReturn->type = BOOL_TYPE;
    while(args->type != NULL_TYPE){
        Value *evalCar = eval(car(args), frame);
        if(evalCar->type != BOOL_TYPE){
            orReturn->i = 1;
            return orReturn;
        } else {
            if(evalCar->i == 1){
                orReturn->i = 1;
                return orReturn;
            }
        }
        args = cdr(args);
    }
    orReturn->i = 0;
    return orReturn;
}

// evaluates begin statements
Value *evalBegin(Value *args, Frame *frame){
    Value *beginReturn = talloc(sizeof(Value));
    if(args->type == NULL_TYPE){
        beginReturn->type = VOID_TYPE;
        return beginReturn;
    }
    Value *evalCar = talloc(sizeof(Value));
    while(args->type != NULL_TYPE){
        evalCar = eval(car(args), frame);
        args = cdr(args);
    }
    return evalCar;
}


//applies a function to given arguments
Value *apply(Value *function, Value *args){
    if(function->type == CLOSURE_TYPE){
        Frame *functionFrame = talloc(sizeof(Frame));
        functionFrame -> parent = function->cl.frame;
        functionFrame -> bindings = makeNull();

        Value *params = function->cl.paramNames;


        while(params->type != NULL_TYPE){
            functionFrame->bindings = cons(cons(car(params), car(args)), functionFrame->bindings);
            args = cdr(args);
            params = cdr(params);
        }

        return eval(function->cl.functionCode, functionFrame);
    }
    else if(function->type == PRIMITIVE_TYPE){
        return (*function->pf)(args);
    }
    else{
        printf("Evaluation error: function is not a primitive or closure type");
        texit(1);
    }
    return args; //not possible to reach, simply here to avoid warning
}

//evaluates each argument and returns the list of evaluated args
Value *evalEach(Value *args, Frame *frame){
    Value *evalArgs = makeNull();
    while(args->type != NULL_TYPE){
        evalArgs = cons(eval(car(args), frame), evalArgs);
        args = cdr(args);
    }
    return reverse(evalArgs);
}

//evaluates built in car
Value *builtInCar(Value *args) {
    if(car(args)->type != CONS_TYPE){
        printf("Evaluation error: car must take in a list in the first argument\n");
        texit(1);
    }
    if(cdr(args)->type != NULL_TYPE){
        printf("Evaluation error: too many arguments\n");
        texit(1);
    }
    //Three cars to get the original car
    return car(car(car(args)));
}

//evaluates built in cdr
Value *builtInCdr(Value *args) {

    if(args->type == NULL_TYPE){
        printf("Evaluation error: no arguments to cdr\n");
        texit(1);
    }

    // if we have the cdr of a dotted list, take what is after the dot
    if(args->type != NULL_TYPE){
        if(car(args)->type != NULL_TYPE){
            if(car(car(args))->type != NULL_TYPE){
                if(cdr(car(car(args)))->type != NULL_TYPE){
                    if(car(cdr(car(car(args))))->type != NULL_TYPE){
                        if(car(cdr(car(car(args))))->type == STR_TYPE){
                            if(!strcmp(car(cdr(car(car(args))))->s, ".")){
                                return cdr(cdr(car(car(args))));
                            }
                        }
                    }
                }
            }
        }
    }
    
    
   return cons(cdr(car(car(args))), makeNull());
}

//evaluates built in null?
Value *builtInNull(Value *args) {

    if(args->type == NULL_TYPE){
        printf("Evaluation error: no arguments to null?\n");
        texit(1);
    }
    if(cdr(args)->type != NULL_TYPE){
        printf("Evaluation error: too many arguments to null?\n");
        texit(1);
    }
    Value *nullVal = talloc(sizeof(Value));
    nullVal->type = BOOL_TYPE;
    while(args->type == CONS_TYPE){
        args = car(args);
    }
    if(isNull(args)){
        nullVal->i = 1;
    }
    else { 
        nullVal->i = 0;
    }
    return nullVal;
}

//evaluates built in add
Value *builtInAdd(Value *args) {
    Value *tempArgs = args;
    bool doubleFound = false;
    Value *addReturn = talloc(sizeof(Value));

    while(tempArgs->type != NULL_TYPE){
        if(car(tempArgs)->type == DOUBLE_TYPE){
            doubleFound = true;
            break;
        }
        tempArgs = cdr(tempArgs);
    }

    if(doubleFound){
        double doubleSum = 0;
        while(args->type != NULL_TYPE){
            if(car(args)->type == INT_TYPE){
                doubleSum += car(args)->i;
            }
            else if(car(args)->type == DOUBLE_TYPE){
                doubleSum += car(args)->d;
            }
            else{
                printf("Evaluation error: cannot add non int or double types\n");
                texit(1);
            }
            args = cdr(args);
        }
        addReturn->type = DOUBLE_TYPE;
        addReturn->d = doubleSum;
    } else {
        int intSum = 0;
        while(args->type != NULL_TYPE){
            if(car(args)->type == INT_TYPE){
                intSum += car(args)->i;
            }
            else{
                printf("Evaluation error: cannot add non int or double types\n");
                texit(1);
            }
            args = cdr(args);
        }
        addReturn->type = INT_TYPE;
        addReturn->i = intSum;
    }
    return addReturn;
}

//implements built in minus
Value *builtInMinus(Value *args) {

    if(args->type == NULL_TYPE || car(args)->type == NULL_TYPE || 
        cdr(args)->type == NULL_TYPE || cdr(cdr(args))->type != NULL_TYPE){
        printf("Evaluation error: wrong number of arguments to minus\n");
        texit(1);
    }

    if(car(args)->type != DOUBLE_TYPE && car(args)->type != INT_TYPE){
        printf("Evaluation error: first argument is not an int or double\n");
        texit(1);
    }
    if(car(cdr(args))->type != DOUBLE_TYPE && car(cdr(args))->type != INT_TYPE){
        printf("Evaluation error: second argument is not an int or double\n");
        texit(1);
    }

    Value *minusReturn = talloc(sizeof(Value));
    if(car(args)->type == DOUBLE_TYPE || car(cdr(args))->type == DOUBLE_TYPE){
        minusReturn->type = DOUBLE_TYPE;
        double minusReturnDouble = 0;
        if(car(args)->type == DOUBLE_TYPE && car(cdr(args))->type != DOUBLE_TYPE){
            minusReturnDouble = car(args)->d - car(cdr(args))->i;
        } else if(car(args)->type != DOUBLE_TYPE && car(cdr(args))->type == DOUBLE_TYPE){
            minusReturnDouble = car(args)->i - car(cdr(args))->d;
        } else {
            minusReturnDouble = car(args)->d - car(cdr(args))->d;
        }
        minusReturn->d = minusReturnDouble;      
    } else {
        minusReturn->type = INT_TYPE;
        int minusReturnInt = car(args)->i - car(cdr(args))->i;
        minusReturn->i = minusReturnInt;
    }

    return minusReturn;
}



//evaluates built in multiply
Value *builtInMultiply(Value *args) {
    Value *tempArgs = args;
    bool doubleFound = false;
    Value *multiplyReturn = talloc(sizeof(Value));

    while(tempArgs->type != NULL_TYPE){
        if(car(tempArgs)->type == DOUBLE_TYPE){
            doubleFound = true;
            break;
        }
        tempArgs = cdr(tempArgs);
    }

    if(doubleFound){
        double doubleMult = 1;
        while(args->type != NULL_TYPE){
            if(car(args)->type == INT_TYPE){
                doubleMult *= car(args)->i;
            }
            else if(car(args)->type == DOUBLE_TYPE){
                doubleMult *= car(args)->d;
            }
            else{
                printf("Evaluation error: cannot multiply non int or double types\n");
                texit(1);
            }
            args = cdr(args);
        }
        multiplyReturn->type = DOUBLE_TYPE;
        multiplyReturn->d = doubleMult;
    } else {
        int intMult = 1;
        while(args->type != NULL_TYPE){
            if(car(args)->type == INT_TYPE){
                intMult *= car(args)->i;
            }
            else{
                printf("Evaluation error: cannot multiply non int or double types\n");
                texit(1);
            }
            args = cdr(args);
        }
        multiplyReturn->type = INT_TYPE;
        multiplyReturn->i = intMult;
    }
    return multiplyReturn;
}


//implements built in divide
Value *builtInDivide(Value *args) {

    if(args->type == NULL_TYPE || car(args)->type == NULL_TYPE || 
        cdr(args)->type == NULL_TYPE || cdr(cdr(args))->type != NULL_TYPE){
        printf("Evaluation error: wrong number of arguments to divide\n");
        texit(1);
    }

    if(car(args)->type != DOUBLE_TYPE && car(args)->type != INT_TYPE){
        printf("Evaluation error: first argument is not an int or double\n");
        texit(1);
    }
    if(car(cdr(args))->type != DOUBLE_TYPE && car(cdr(args))->type != INT_TYPE){
        printf("Evaluation error: second argument is not an int or double\n");
        texit(1);
    }

    Value *divideReturn = talloc(sizeof(Value));
    if(car(args)->type == DOUBLE_TYPE || car(cdr(args))->type == DOUBLE_TYPE){
        divideReturn->type = DOUBLE_TYPE;
        double divideReturnDouble = 0;
        if(car(args)->type == DOUBLE_TYPE && car(cdr(args))->type != DOUBLE_TYPE){
            divideReturnDouble = car(args)->d / car(cdr(args))->i;
        } else if(car(args)->type != DOUBLE_TYPE && car(cdr(args))->type == DOUBLE_TYPE){
            divideReturnDouble = car(args)->i / car(cdr(args))->d;
        } else {
            divideReturnDouble = car(args)->d / car(cdr(args))->d;
        }
        divideReturn->d = divideReturnDouble;      
    } else {
        if(car(args)->i % car(cdr(args))->i == 0){
            divideReturn->type = INT_TYPE;
            int divideReturnInt = car(args)->i / car(cdr(args))->i;
            divideReturn->i = divideReturnInt;
        }
        else{
            divideReturn->type = DOUBLE_TYPE;
            double minusReturnDouble = (double) car(args)->i / car(cdr(args))->i;
            divideReturn->d = minusReturnDouble;
        }
        
    }

    return divideReturn;
}


//implements built in modulo
Value *builtInModulo(Value *args) {
    if(args->type == NULL_TYPE || car(args)->type == NULL_TYPE || 
        cdr(args)->type == NULL_TYPE || cdr(cdr(args))->type != NULL_TYPE){
        printf("Evaluation error: wrong number of arguments to modulo\n");
        texit(1);
    }

    if(car(args)->type != INT_TYPE || car(cdr(args))->type != INT_TYPE){
        printf("Evaluation error: arguments must be an int or double\n");
        texit(1);
    }

    Value *modReturn = talloc(sizeof(Value));
    modReturn->type = INT_TYPE;
    modReturn->i = car(args)->i % car(cdr(args))->i;
    return modReturn;

}

//implements built in less than
Value *builtInLessThan(Value *args) {
    if(args->type == NULL_TYPE || car(args)->type == NULL_TYPE || 
        cdr(args)->type == NULL_TYPE || cdr(cdr(args))->type != NULL_TYPE){
        printf("Evaluation error: wrong number of arguments to less than\n");
        texit(1);
    }

    if(car(args)->type != DOUBLE_TYPE && car(args)->type != INT_TYPE){
        printf("Evaluation error: first argument is not an int or double\n");
        texit(1);
    }
    if(car(cdr(args))->type != DOUBLE_TYPE && car(cdr(args))->type != INT_TYPE){
        printf("Evaluation error: second argument is not an int or double\n");
        texit(1);
    }

    Value *lessThanReturn = talloc(sizeof(Value));
    lessThanReturn->type = BOOL_TYPE;
    if(car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == DOUBLE_TYPE){
        lessThanReturn->i = (car(args)->d < car(cdr(args))->d);
    } else if(car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == INT_TYPE){
        lessThanReturn->i = (car(args)->d < car(cdr(args))->i);
    } else if(car(args)->type == INT_TYPE && car(cdr(args))->type == DOUBLE_TYPE){
        lessThanReturn->i = (car(args)->i < car(cdr(args))->d);
    } else {
        lessThanReturn->i = (car(args)->i < car(cdr(args))->i);
    }
    return lessThanReturn;
}


//implements built in greater than
Value *builtInGreaterThan(Value *args) {
    if(args->type == NULL_TYPE || car(args)->type == NULL_TYPE || 
        cdr(args)->type == NULL_TYPE || cdr(cdr(args))->type != NULL_TYPE){
        printf("Evaluation error: wrong number of arguments to greater than\n");
        texit(1);
    }

    if(car(args)->type != DOUBLE_TYPE && car(args)->type != INT_TYPE){
        printf("Evaluation error: first argument is not an int or double\n");
        texit(1);
    }
    if(car(cdr(args))->type != DOUBLE_TYPE && car(cdr(args))->type != INT_TYPE){
        printf("Evaluation error: second argument is not an int or double\n");
        texit(1);
    }

    Value *greaterThanReturn = talloc(sizeof(Value));
    greaterThanReturn->type = BOOL_TYPE;
    if(car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == DOUBLE_TYPE){
        greaterThanReturn->i = (car(args)->d > car(cdr(args))->d);
    } else if(car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == INT_TYPE){
        greaterThanReturn->i = (car(args)->d > car(cdr(args))->i);
    } else if(car(args)->type == INT_TYPE && car(cdr(args))->type == DOUBLE_TYPE){
        greaterThanReturn->i = (car(args)->i > car(cdr(args))->d);
    } else {
        greaterThanReturn->i = (car(args)->i > car(cdr(args))->i);
    }
    return greaterThanReturn;
}

//implements built in equals
Value *builtInEquals(Value *args) {
    if(args->type == NULL_TYPE || car(args)->type == NULL_TYPE || 
        cdr(args)->type == NULL_TYPE || cdr(cdr(args))->type != NULL_TYPE){
        printf("Evaluation error: wrong number of arguments to equals\n");
        texit(1);
    }
    
    if(car(args)->type != DOUBLE_TYPE && car(args)->type != INT_TYPE){
        printf("Evaluation error: first argument is not an int or double\n");
        texit(1);
    }
    if(car(cdr(args))->type != DOUBLE_TYPE && car(cdr(args))->type != INT_TYPE){
        printf("Evaluation error: second argument is not an int or double\n");
        texit(1);
    }

    Value *equalsReturn = talloc(sizeof(Value));
    equalsReturn->type = BOOL_TYPE;
    if(car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == DOUBLE_TYPE){
        equalsReturn->i = (car(args)->d == car(cdr(args))->d);
    } else if(car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == INT_TYPE){
        equalsReturn->i = (car(args)->d == (double) car(cdr(args))->i);
    } else if(car(args)->type == INT_TYPE && car(cdr(args))->type == DOUBLE_TYPE){
        equalsReturn->i = ((double) car(args)->i == car(cdr(args))->d);
    } else {
        equalsReturn->i = (car(args)->i == car(cdr(args))->i);
    }
    return equalsReturn;
}



//implements cons
Value *builtInCons(Value *args){
    if(args->type == NULL_TYPE){
        printf("Evaluation error: no to cons\n");
        texit(1);
    }
    if(cdr(args)->type == NULL_TYPE){
        printf("Evaluation error: too few arguments to cons\n");
        texit(1);
    }
    if(cdr(cdr(args))->type != NULL_TYPE){
        printf("Evaluation error: too many arguments to cons\n");
        texit(1);
    }
    Value *retValue = talloc(sizeof(Value));

    //case when second item is a list
    if(car(cdr(args))->type == CONS_TYPE){
        //case when we have list of list
        if(car(args)->type == CONS_TYPE){
            if(car(car(args))->type == CONS_TYPE){
                retValue = cons(car(car(args)), car(car(cdr(args))));
            } else {
                retValue = cons(car(args), car(car(cdr(args))));
            }
        }
        else {
            retValue = cons(car(args), car(car(cdr(args))));
        }
        return cons(retValue, makeNull()); 
    }
    //case when second item isn't list, make dotted pair
    else{
        Value *dotVal = talloc(sizeof(Value));
        dotVal->type = STR_TYPE;
        char *dot = talloc(sizeof(char) * 2);
        dot[0] = '.';
        dot[1] = '\0';
        dotVal->s = talloc(sizeof(char) * 2);
        strcpy(dotVal->s, dot);
        Value *newCdr = cons(car(cdr(args)), makeNull());
        
        retValue = cons(dotVal, newCdr);
        retValue = cons(car(args), retValue);
        return cons(retValue, makeNull());
    }
    return args;
    
}


//binds a primitive function
void bindPrimitiveFunction(char *name, Value *(*function)(struct Value *), Frame *frame) {
    // Bind 'name' to 'function' in 'frame'
    Value *value = talloc(sizeof(Value));
    value->type = PRIMITIVE_TYPE;
    value->pf = function;

    Value *functionName = talloc(sizeof(Value));
    functionName->type = STR_TYPE;
    functionName->s = name;
    frame->bindings = cons(cons(functionName, value), frame->bindings);
}

//evaluates a node
Value *eval(Value *tree, Frame *frame) {
    switch (tree->type)  {
        case NULL_TYPE: {
            break;
        }
        case PTR_TYPE: {
            break;
        }
        case INT_TYPE: {
            return tree;
            break;
        }
        case DOUBLE_TYPE: {
            return tree;
            break;
        }
        case BOOL_TYPE: {
            return tree;
            break;
        }
        case STR_TYPE:{
            return tree;
            break;
        }
        case SYMBOL_TYPE: {
            return lookUpSymbol(tree, frame);
            break;
        }  
        case CONS_TYPE: {
            Value *first = car(tree);
            Value *args = cdr(tree);

            if (!strcmp(first->s, "if")) {
                return evalIf(args, frame);
            }
            else if (!strcmp(first->s, "let")) {
                return evalLet(args, frame);
            }
            else if (!strcmp(first->s, "quote")) {
                return evalQuote(args);
            }
            else if (!strcmp(first->s, "define")) {
                return evalDefine(args, frame);
            }
            else if (!strcmp(first->s, "lambda")) {
                return evalLambda(args, frame);
            }
            else if (!strcmp(first->s, "and")) {
                return evalAnd(args, frame);
            }
            else if (!strcmp(first->s, "or")) {
                return evalOr(args, frame);
            }
            else if (!strcmp(first->s, "begin")) {
                return evalBegin(args, frame);
            }
            else if (!strcmp(first->s, "let*")) {
                return evalLetStar(args, frame);
            }
            else if (!strcmp(first->s, "letrec")) {
                return evalLetRec(args, frame);
            }
            else if (!strcmp(first->s, "cond")) {
                return evalCond(args, frame);
            }
            else if (!strcmp(first->s, "set!")) {
                return evalSet(args, frame, frame);
            }

            // Other special forms go here...

            else {
                // If it's not a special form, evaluate 'first', evaluate the args, then
                // apply 'first' on the args.
                Value *evaluatedOperator = eval(first, frame);
                Value *evaluatedArgs = evalEach(args, frame);
                return apply(evaluatedOperator, evaluatedArgs);
            }
            break;
        }
        default:
            ;
    }
    return tree; // to prevent compiler warning about non-void function
}

//interprets a value node
void interpret(Value *tree){
    Frame *globalFrame = talloc(sizeof(Frame));
    globalFrame -> parent = NULL;
    globalFrame -> bindings = makeNull();

    // Create bindings in the global frame for all of
    // the built-in functions.
    bindPrimitiveFunction("car", &builtInCar, globalFrame);
    bindPrimitiveFunction("cdr", &builtInCdr, globalFrame);
    bindPrimitiveFunction("null?", &builtInNull, globalFrame);
    bindPrimitiveFunction("+", &builtInAdd, globalFrame);
    bindPrimitiveFunction("cons", &builtInCons, globalFrame);

    bindPrimitiveFunction("-", &builtInMinus, globalFrame);
    bindPrimitiveFunction("*", &builtInMultiply, globalFrame);
    bindPrimitiveFunction("/", &builtInDivide, globalFrame);
    bindPrimitiveFunction("modulo", &builtInModulo, globalFrame);
    bindPrimitiveFunction("<", &builtInLessThan, globalFrame);
    bindPrimitiveFunction(">", &builtInGreaterThan, globalFrame);
    bindPrimitiveFunction("=", &builtInEquals, globalFrame);

    while (tree->type != NULL_TYPE){
        Value *evalResult = eval(car(tree), globalFrame);
        printValue(evalResult);
        tree = cdr(tree);
    }
}

#endif

