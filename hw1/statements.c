#include <stdio.h>
#include <string.h>
#include <ctype.h>

enum state {COUNT, BRACE, BRACE2, STRING, CHARACTER, PARENTHESIS, COMMENT, COMMENT1, COMMENT2, COMMENT2_1};


int main(){
    int count_colons = 0;
    int count_braces = 0;
    enum state curr = COUNT;
    enum state next = COUNT;

    int paren_count = 0;
    int newline_count = 0;
    
    //read inputs
    char character;
    while(scanf("%c", &character)!= EOF){
        switch (curr){
            case COUNT:
                if(character == '<'){ // <%
                    curr = BRACE;
                }
                if(character == '\"'){ //STRING
                    curr = STRING;
                }
                if(character == '\''){ //CHARACTER
                    curr = CHARACTER;
                }
                if(character == '('){ //PARENTHESIS
                    paren_count++;
                    curr = PARENTHESIS;
                }
                if(character == '/'){ //COMMENT
                    curr = COMMENT;
                }

                //count
                if(character == ';'){
                    count_colons++;
                }
                if(character == '{'){
                    count_braces++;
                }

                break;

            case BRACE:
                if(character == '%'){ // <%
                    curr = COUNT;
                    count_braces++;
                }
                curr = COUNT;
                break;

            case STRING:
                if(character == '\"'){ //STRING
                    curr = next;
                    next = COUNT;
                }
                break;

            case CHARACTER:
                if(character == '\''){ //CHARACTER
                    curr = next;
                    next = COUNT;
                }
                break;
            
            case PARENTHESIS:
                if(character == '\"'){ //STRING
                    curr = STRING;
                    next = PARENTHESIS;
                }
                if(character == '\''){ //CHARACTER
                    curr = CHARACTER;
                    next = PARENTHESIS;
                }
                if(character == '('){ //Function call
                    paren_count++;
                }
                if(character == ')'){ //right ")"
                    paren_count = paren_count - 1;
                    if(paren_count == 0){
                        curr = COUNT;
                    }
                }
                break;

            case COMMENT:
                if(character == '/'){ //COMMENT
                    curr = COMMENT1;
                }
                if(character == '*'){ //COMMENT
                    curr = COMMENT2;
                }
                break;

            case COMMENT1: //comment format is"//"
                if(character == '\\'){
                    newline_count++;
                }
                if(character == '\n'){
                    if(newline_count == 0){
                        curr = COUNT;
                    }
                    newline_count = newline_count - 1;
                }
                break;

            case COMMENT2: //comment format is"/*...*/"
                if(character == '*'){
                    curr = COMMENT2_1;
                }
                break;
            case COMMENT2_1: //comment format is"/*...*/"
                if(character == '/'){
                    curr = COUNT;
                }
                else{
                    curr = COMMENT2;
                }
                break;
        }
    }
    
    int total = count_colons + count_braces;
    printf("%d\n", total);
    return 0;
}