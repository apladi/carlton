/* oilscript, the self-centred-named language */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>

typedef enum {
    TYPE_ADD, // Addition
    TYPE_SUB, // Subtraction
    TYPE_MPY, // Multiplication
    TYPE_DIV, // Division
    TYPE_GRT, // Greater than
    TYPE_LWR, // Lower than
    TYPE_QTM, // Quotations (UNUSED)
    TYPE_STR, // String
    TYPE_INT, // Integer
    TYPE_SET, // Set (Equals)
    TYPE_END, // Semi-colon, line end 
    TYPE_OPR, // Open parenthesis
    TYPE_CPR, // Close parenthesis
    TYPE_IDN, // Identifier
    TYPE_CMA  // Comma
} type_t;

typedef struct {
    type_t type;
    char   data[256];
} token_t;

typedef struct {
    int     rank;
    token_t token;
} parse_t;

const char *str_type(type_t type) {
    switch (type) {
        case  0: return "ADD"; break;
        case  1: return "SUB"; break;
        case  2: return "MPY"; break;
        case  3: return "DIV"; break;
        case  4: return "GTR"; break;
        case  5: return "LWR"; break;
        case  6: return "QTM"; break;
        case  7: return "STR"; break;
        case  8: return "INT"; break;
        case  9: return "SET"; break;
        case 10: return "END"; break;
        case 11: return "OPR"; break;
        case 12: return "CPR"; break;
        case 13: return "IDN"; break;
        case 14: return "CMA"; break;
    }
}

int main(void) {
    parse_t parsed[256];
    token_t tokens[256];
    token_t token;
    char *input;
    int case_for = 0;
    int counter  = 0;
    int check    = 0;

    input = readline("--> ");

    for (int i = 0; i < strlen(input); ++i) {
        check = 0;
        memset(token.data, 0, sizeof(token.data));

        switch (input[i]) {
            case '+':
                token.type    = TYPE_ADD;
                token.data[0] = '+';
                token.data[1] = '\0';
                break;
            case '-':
                token.type    = TYPE_SUB;
                token.data[0] = '-';
                token.data[1] = '\0';
                break;
            case '*':
                token.type    = TYPE_MPY;
                token.data[0] = '*';
                token.data[1] = '\0';
                break;
            case '/':
                token.type    = TYPE_DIV;
                token.data[0] = '/';
                token.data[1] = '\0';
                break;
            case '>':
                token.type    = TYPE_GRT;
                token.data[0] = '>';
                token.data[1] = '\0';
                break;
            case '<':
                token.type    = TYPE_LWR;
                token.data[0] = '<';
                token.data[1] = '\0';
                break;
            case '"':
                token.type = TYPE_STR;
                case_for   = 0;
                ++i;
                for (i; i < strlen(input); ++i) {
                    if (input[i] == '"') {
                        tokens[counter] = token;
                        ++counter;
                        break;
                    } else {
                        token.data[case_for] = input[i];
                        ++case_for;
                    }
                }

                check = 1;
                break;
            case '=':
                token.type    = TYPE_SET;
                token.data[0] = '=';
                token.data[1] = '\0';
                break;
            case ';':
                token.type    = TYPE_END;
                token.data[0] = ';';
                token.data[1] = '\0';
                break;
            case '(':
                token.type    = TYPE_OPR;
                token.data[0] = '(';
                token.data[1] = '\0';
                break;
            case ')':
                token.type    = TYPE_CPR;
                token.data[0] = ')';
                token.data[1] = '\0';
                break;
            case ',':
                token.type    = TYPE_CMA;
                token.data[0] = ',';
                token.data[1] = '\0';
                break;
            default:
                if (isdigit(input[i])) {
                    token.type = TYPE_INT;
                    case_for   = 0;

                    for (i; i < strlen(input); ++i) {
                        if (!isdigit(input[i])) {
                            tokens[counter] = token;
                            ++counter;

                            --i;
                            break;
                        } else {
                            token.data[case_for] = input[i];
                            ++case_for;
                        }
                    }

                    if (i == strlen(input)) {
                        tokens[counter] = token;
                        ++counter;
                    }

                    check = 1;
                } else if (isalpha(input[i])) {
                    token.type = TYPE_IDN;
                    case_for   = 0;

                    for (i; i < strlen(input); ++i) {
                        if (!isalpha(input[i])) {
                            tokens[counter] = token;
                            ++counter;

                            --i;
                            break;
                        } else {
                            token.data[case_for] = input[i];
                            ++case_for;
                        }
                    }

                    if (i == strlen(input)) {
                        tokens[counter] = token;
                        ++counter;
                    }

                    check = 1;
                } else {
                    check = 1;
                }

                break;
        }

        if (!check) {
            tokens[counter] = token;
            ++counter;
        }    
    }

    /* PARSER (BROKEN) */ 
    // int rank = 0;

    // for (int i = 0; i < counter; ++i) {
    //     if (tokens[i].type == TYPE_END) {
    //         break;
    //     }

    //     printf("%i|", rank);
    //     parsed[i].token = tokens[i];

    //     for (int x = 0; x < rank; ++x) {
    //         putchar(32);
    //     }

    //     printf("%s-%s\n", str_type(parsed[i].token.type), parsed[i].token.data);
    // }

    free(input);
}