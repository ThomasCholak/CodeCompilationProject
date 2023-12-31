#include <iostream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <set>

#include "P4.h"

std::vector<Variable> variables;

// lexer function to identify keywords and variables names
std::vector<Token> lexer(const std::string& input) {
    std::istringstream iss(input);
    std::vector<Token> tokens;

    std::string token;
    while (iss >> token) {

        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
        Token currentToken;

        if (token.size() > 1 && token[0] == '#' && token[token.size() - 1] == '$') {
            continue;
        }

        // functions to identify keywords
        if (token == "let") {
            currentToken = {"LETtk", ""};
        } else if (token == "print") {
            currentToken = {"PRINTtk", ""};
        } else if (token == "scan") {
            currentToken = {"SCANtk", ""};
        } else if (token == "start") {
            currentToken = {"STARTtk", ""};
        } else if (token == "stop") {
            currentToken = {"STOPtk", ""};
        } else if (token == "cond") {
            currentToken = {"CONDtk", ""};
        } else if (token == "loop") {
            currentToken = {"LOOPtk", ""};
        } else if (token == "main") {
            currentToken = {"MAINtk", ""};
        } else if (token == "end") {
            currentToken = {"EOFtk", ""};
        } else {

            bool isInteger = true;
            for (char c : token) {
                if (!std::isdigit(c)) {
                    isInteger = false;  // function for identifying integers
                    break;
                }
            }
            // below code is to identify operands
            if (isInteger) {
                currentToken = {"INTtk", token};
            } else if (token == "~") {
                currentToken = {"TILDEtk", ""};
            } else if (token == "+") {
                currentToken = {"PLUStk", ""};
            } else if (token == "-") {
                currentToken = {"MINUStk", ""};
            } else if (token == "<") {
                currentToken = {"LESStk", ""};
            } else if (token == ">") {
                currentToken = {"GREATtk", ""};
            } else if (token == ">=") {
                currentToken = {"GREATEQtk", ""};
            } else if (token == "<=") {
                currentToken = {"LESSEQtk", ""};
            } else if (token == "/") {
                currentToken = {"DIVtk", ""};
            } else if (token == "*") {
                currentToken = {"MULTtk", ""};
            } else if (token == "=") {
                currentToken = {"EQUALtk", ""};
            } else if (token == ".") {
                currentToken = {"PERIODtk", ""};
            } else if (token == "(") {
                currentToken = {"OPEN_PARAtk", ""};
            } else if (token == ")") {
                currentToken = {"CLOSE_PARAtk", ""};
            } else if (token == "[") {
                currentToken = {"OPEN_BRACKETtk", ""};
            } else if (token == "]") {
                currentToken = {"CLOSE_BRACKETtk", ""};
            } else {
                currentToken = {"VARIABLEtk", token};
            }
        }
        tokens.push_back(currentToken);
    }
    return tokens;
}

size_t currentIndex = 0;

// initializes the beginning of the BNF structure
TreeNode* program(const std::vector<Token>& tokens) {
    int variableCount = 0;
    int lineNumber = 1;
    TreeNode *node = new TreeNode("<program>");

    std::set<std::string> declaredVariables;

    node->left = vars(tokens, variableCount, lineNumber, declaredVariables);
    node->right = stats(tokens, variableCount, lineNumber, declaredVariables);

    return node;
}

void consumeToken(const std::vector<Token>& tokens) {
    if (currentIndex < tokens.size() - 1) {
        if (tokens[currentIndex].type == "EOFtk") {
            std::cout << "Error: Unexpected token 'EOFtk'." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        currentIndex++;
    } else {
        std::cout << "Error: Token number exceeded." << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

TreeNode* vars(const std::vector<Token>& tokens, int& variableCount, int& lineNumber, std::set<std::string>& declaredVariables) {
    TreeNode *node = new TreeNode("<vars>");

    if (tokens[currentIndex].type == "LETtk") {
        consumeToken(tokens);
        node->left = varList(tokens, variableCount, lineNumber, declaredVariables);
    }

    return node;
}

TreeNode* varList(const std::vector<Token>& tokens, int& variableCount, int& lineNumber, std::set<std::string>& declaredVariables) {
    TreeNode *node = new TreeNode("<varList>");

    if (tokens[currentIndex].type == "VARIABLEtk") {
        std::string variableName = tokens[currentIndex].value;

        if (declaredVariables.find(variableName) != declaredVariables.end() || variableName.empty()) {
            std::cout << "Error: Variable '" << variableName << "' already declared on line " << lineNumber << std::endl;
            std::exit(EXIT_FAILURE);
        }
        consumeToken(tokens);

        if (tokens[currentIndex].type == "EQUALtk") {
            consumeToken(tokens);

            if (tokens[currentIndex].type == "INTtk") {
                std::string variableValue = tokens[currentIndex].value;
                consumeToken(tokens);

                declaredVariables.insert(variableName);

                node->value = "<varList>";
                node->variableValue = variableName + " " + variableValue;
                node->variableCount = ++variableCount;
                node->lineNumber = lineNumber;
                node->right = varList(tokens, variableCount, lineNumber, declaredVariables);
            } else {
                std::cout << "ERROR: must have an '=' after a variable " << lineNumber << std::endl;
            }
        } else {
            std::cout << "ERROR: must have an '=' after a variable " << lineNumber << std::endl;
        }
    }

    return node;
}

TreeNode* exp(const std::vector<Token>& tokens, int& variableCount, int& lineNumber) {
    TreeNode* node = new TreeNode("<exp>");

    node->left = M(tokens, variableCount, lineNumber);

    if (tokens[currentIndex].type == "PLUStk" || tokens[currentIndex].type == "MINUStk") {
        node->value = tokens[currentIndex].value;
        consumeToken(tokens);
        node->right = exp(tokens, variableCount, lineNumber);
    }

    return node;
}

TreeNode* M(const std::vector<Token>& tokens, int& variableCount, int& lineNumber) {
    TreeNode* node = new TreeNode("<M>");

    node->left = N(tokens, variableCount, lineNumber);

    if (tokens[currentIndex].type == "MULTtk") {
        node->value = tokens[currentIndex].value;
        consumeToken(tokens);
        node->right = M(tokens, variableCount, lineNumber);
    }

    return node;
}

TreeNode* N(const std::vector<Token>& tokens, int& variableCount, int& lineNumber) {
    TreeNode* node = new TreeNode("<N>");

    if (tokens[currentIndex].type == "MINUStk") {
        node->value = tokens[currentIndex].value;
        consumeToken(tokens);
        node->left = N(tokens, variableCount, lineNumber);
    } else {

        node->left = R(tokens, variableCount, lineNumber);

        if (tokens[currentIndex].type == "DIVtk") {
            node->value = tokens[currentIndex].value;
            consumeToken(tokens);
            node->right = N(tokens, variableCount, lineNumber);
        }
    }

    return node;
}

TreeNode* R(const std::vector<Token>& tokens, int& variableCount, int& lineNumber) {
    TreeNode* node = new TreeNode("<R>");

    if (tokens[currentIndex].type == "OPEN_BRACKETtk") {
        consumeToken(tokens);
        node->left = exp(tokens, variableCount, lineNumber);

        if (tokens[currentIndex].type == "CLOSE_BRACKETtk") {
            consumeToken(tokens);
        } else {
            std::cout << "Error: Expected ']' on line " << lineNumber << std::endl;
        }
    } else if (tokens[currentIndex].type == "VARIABLEtk" || tokens[currentIndex].type == "INTtk") {
        node->value = "<R>";
        node->variableValue = tokens[currentIndex].value;
        consumeToken(tokens);
    } else if (tokens[currentIndex].type == "PLUStk" || tokens[currentIndex].type == "MINUStk" ||
               tokens[currentIndex].type == "MULTtk" || tokens[currentIndex].type == "DIVtk") {
        node->value = tokens[currentIndex].value;
        consumeToken(tokens);
        node->left = R(tokens, variableCount, lineNumber);
        node->right = R(tokens, variableCount, lineNumber);
    } else {
        std::cout << "Error: Unexpected token on line " << lineNumber << std::endl;
    }

    return node;
}

TreeNode* stats(const std::vector<Token>& tokens, int& variableCount, int& lineNumber, std::set<std::string>& declaredVariables) {
    TreeNode *node = new TreeNode("<stats>");

    if (tokens[currentIndex].type == "PERIODtk") {
        consumeToken(tokens);
    }
    if (tokens[currentIndex].type == "MAINtk") {
        consumeToken(tokens);
        node->left = stat(tokens, variableCount, lineNumber, declaredVariables);
        node->right = mStat(tokens, variableCount, lineNumber, declaredVariables);
    } else if (tokens[currentIndex].type == "EOFtk")
        consumeToken(tokens);

    return node;
}

TreeNode* mStat(const std::vector<Token>& tokens, int& variableCount, int& lineNumber, std::set<std::string>& declaredVariables) {
    TreeNode* node = new TreeNode("<mStat>");

    if (tokens[currentIndex].type != "EOFtk" && tokens[currentIndex].type != "STOPtk") {
        node->left = stat(tokens, variableCount, lineNumber, declaredVariables);
        node->right = mStat(tokens, variableCount, lineNumber, declaredVariables);
    }

    return node;
}

TreeNode* stat(const std::vector<Token>& tokens, int& variableCount, int& lineNumber, std::set<std::string>& declaredVariables) {
    TreeNode* node = new TreeNode("<stat>");

    if (tokens[currentIndex].type == "SCANtk") {
        node->left = in(tokens, variableCount, lineNumber);
    } else if (tokens[currentIndex].type == "PRINTtk") {
        node->left = out(tokens, variableCount, lineNumber);
    } else if (tokens[currentIndex].type == "VARIABLEtk") {
        node->left = assign(tokens, variableCount, lineNumber);
    } else if (tokens[currentIndex].type == "STARTtk") {
        node->left = block(tokens, variableCount, lineNumber, declaredVariables);
    } else if (tokens[currentIndex].type == "CONDtk") {
        node->left = ifStatement(tokens, variableCount, lineNumber, declaredVariables);
    } else if (tokens[currentIndex].type == "LOOPtk") {
        node->left = loop(tokens, variableCount, lineNumber, declaredVariables);
    }

    return node;
}

TreeNode* block(const std::vector<Token>& tokens, int& variableCount, int& lineNumber, std::set<std::string>& declaredVariables) {
    TreeNode* node = new TreeNode("<block>");

    if (tokens[currentIndex].type == "STARTtk") {
        consumeToken(tokens);

        node->left = vars(tokens, variableCount, lineNumber, declaredVariables);
        node->right = stats(tokens, variableCount, lineNumber, declaredVariables);

        if (tokens[currentIndex].type == "STOPtk")
            consumeToken(tokens);
    } else {
        std::cout << "Error: Expected 'START' keyword on line " << lineNumber << std::endl;
    }

    return node;
}

TreeNode* in(const std::vector<Token>& tokens, int& variableCount, int& lineNumber) {
    TreeNode* node = new TreeNode("<in>");

    if (tokens[currentIndex].type == "SCANtk") {
        consumeToken(tokens);

        if (tokens[currentIndex].type == "VARIABLEtk") {
            node->value = "<in>";
            node->variableValue = tokens[currentIndex].value;
            consumeToken(tokens);
            consumeToken(tokens);
            return node;
        } else {
            std::cout << "Error: Expected variable after 'SCAN' on line " << lineNumber << std::endl;
        }
    } else {
        std::cout << "Error: Expected 'SCAN' keyword on line " << lineNumber << std::endl;
    }

    return nullptr;
}

TreeNode* out(const std::vector<Token>& tokens, int& variableCount, int& lineNumber) {
    TreeNode* node = new TreeNode("<out>");

    if (tokens[currentIndex].type == "PRINTtk") {
        consumeToken(tokens);
        node->value = "<out>";
        node->variableValue = tokens[currentIndex].value;
        node->left = exp(tokens, variableCount, lineNumber);

        if (tokens[currentIndex].type == "PERIODtk") {
            consumeToken(tokens);
            return node;
        } else {
            std::cout << "Error: Expected '.' after 'PRINT' statement on line " << lineNumber << std::endl;
        }
    } else {
        std::cout << "Error: Expected 'PRINT' keyword on line " << lineNumber << std::endl;
    }

    return nullptr;
}

TreeNode* ifStatement(const std::vector<Token>& tokens, int& variableCount, int& lineNumber, std::set<std::string>& declaredVariables) {
    TreeNode* node = new TreeNode("<ifStatement>");

    if (tokens[currentIndex].type == "CONDtk") {
        consumeToken(tokens);

        if (tokens[currentIndex].type == "OPEN_PARAtk") {
            consumeToken(tokens);

            node->left = exp(tokens, variableCount, lineNumber);
            node->right = RO(tokens, variableCount, lineNumber);
            node->extra = exp(tokens, variableCount, lineNumber);

            if (tokens[currentIndex].type == "CLOSE_PARAtk") {
                consumeToken(tokens);

                node->extra->left = stat(tokens, variableCount, lineNumber, declaredVariables);
            } else {
                std::cout << "Error: Expected ')' after condition on line " << lineNumber << std::endl;
            }
        } else {
            std::cout << "Error: Expected '(' after 'COND' on line " << lineNumber << std::endl;
        }
    } else {
        std::cout << "Error: Expected 'COND' keyword on line " << lineNumber << std::endl;
    }

    return node;
}

// structure for identifying the operands in a BNF
TreeNode* loop(const std::vector<Token>& tokens, int& variableCount, int& lineNumber, std::set<std::string>& declaredVariables) {
    TreeNode* node = new TreeNode("<loop>");

    if (tokens[currentIndex].type == "LOOPtk") {
        consumeToken(tokens);
        if (tokens[currentIndex].type == "OPEN_PARAtk") {
            consumeToken(tokens);
            node->left = exp(tokens, variableCount, lineNumber);
            if (tokens[currentIndex].type == "GREATtk" || tokens[currentIndex].type == "GREATEQtk" ||
                tokens[currentIndex].type == "LESStk" || tokens[currentIndex].type == "LESSEQtk" ||
                tokens[currentIndex].type == "EQUALtk" || tokens[currentIndex].type == "TILDEtk") {
                node->value = tokens[currentIndex].type;
                consumeToken(tokens);
                node->right = exp(tokens, variableCount, lineNumber);
                if (tokens[currentIndex].type == "CLOSE_PARAtk") {
                    consumeToken(tokens);
                    if (tokens[currentIndex].type == "STARTtk") {
                        consumeToken(tokens);
                        node->right->right = stats(tokens, variableCount, lineNumber, declaredVariables);
                        if (tokens[currentIndex].type == "STOPtk") {
                            consumeToken(tokens);
                            return node;
                        } else {
                            std::cout << "Error: Expected 'STOP' keyword after loop block on line " << lineNumber << std::endl;
                        }
                    } else {
                        std::cout << "Error: Expected 'START' keyword to begin loop block on line " << lineNumber << std::endl;
                    }
                } else {
                    std::cout << "Error: Expected ')' after loop condition on line " << lineNumber << std::endl;
                }
            } else {
                std::cout << "Error: Expected relational operator in loop condition on line " << lineNumber << std::endl;
            }
        } else {
            std::cout << "Error: Expected '(' after 'LOOP' on line " << lineNumber << std::endl;
        }
    } else {
        std::cout << "Error: Expected 'LOOP' keyword on line " << lineNumber << std::endl;
    }

    return nullptr;
}

TreeNode* assign(const std::vector<Token>& tokens, int& variableCount, int& lineNumber) {
    TreeNode* node = new TreeNode("<assign>");

    if (tokens[currentIndex].type == "VARIABLEtk") {
        std::string variableName = "<in> id." + tokens[currentIndex].value;
        consumeToken(tokens);
        consumeToken(tokens);
        node->left = exp(tokens, variableCount, lineNumber);

        if (tokens[currentIndex].type == "PERIODtk") {
            consumeToken(tokens);
            node->right = new TreeNode(variableName);
            return node;
        } else {
            std::cout << "Error: Expected '.' after 'ASSIGN' statement on line " << lineNumber << std::endl;
        }
    } else {
        std::cout << "Error: Expected variable after 'ASSIGN' keyword on line " << lineNumber << std::endl;
    }

    return nullptr;
}

TreeNode* RO(const std::vector<Token>& tokens, int& variableCount, int& lineNumber) {
    TreeNode* node = new TreeNode("<RO>");

    if (tokens[currentIndex].type == "LESSEQtk" || tokens[currentIndex].type == "GREATEQtk" ||
        tokens[currentIndex].type == "LESStk" || tokens[currentIndex].type == "GREATtk" ||
        tokens[currentIndex].type == "EQUALtk" || tokens[currentIndex].type == "TILDEtk") {
        node->value = tokens[currentIndex].type;
        consumeToken(tokens);
    } else {
        std::cout << "Error: Expected relational operator on line " << lineNumber << std::endl;
    }

    return node;
}

void generateCodePreorder(TreeNode* root, std::ofstream& outputFile) {
    if (root == nullptr) {
        return;
    }

    if (root->value == "<in>") {
        outputFile << "READ " << root->variableValue << std::endl;
    }

    if (root->value == "<varList>") {
        variables.push_back({root->variableValue, ""});
    }

    if (!root->variableValue.empty()) {
        if (root->value == "<out>") {
            outputFile << "WRITE " << root->variableValue << std::endl;
        }
    }

    generateCodePreorder(root->left, outputFile);
    generateCodePreorder(root->right, outputFile);

}

// stores variable names into a global variable vector list
void printGlobalVariables(std::ofstream& outputFile) {
    outputFile << "STOP" << std::endl;
    for (const auto& variable : variables) {
        outputFile << variable.name << " " << variable.value << std::endl;
    }
}

// main function in order to read Linux file input
int main(int argc, char *argv[]) {

    std::ifstream inputFile;
    std::string code;

    if (argc > 2) {
        std::cerr << "ERROR: only one file allowed.\n";
    }
    else if (argc == 2) {
        inputFile.open(argv[1]);

        if (!inputFile.is_open()) {
            std::cerr << "ERROR: could not open File: " << argv[1] << std::endl;
            return 1;
        }
    }
    if (inputFile.is_open()) {

        std::string fileName = argv[1];
        std::ofstream outputFile(fileName + ".asm"); // open the file for writing

        std::ostringstream buffer;
        buffer << inputFile.rdbuf();
        code = buffer.str();

        std::vector<Token> tokens = lexer(code);  // tokenizer
        TreeNode *root = program(tokens);               // creates tree

        generateCodePreorder(root, outputFile);
        printGlobalVariables(outputFile);

        outputFile.close();
    }
    else {
        std::string user_str;
        std::cout << "Enter your input code:" << std::endl;
        std::getline(std::cin, user_str);

        std::ofstream outputFile("a.asm");

        std::vector<Token> tokens = lexer(user_str);  // tokenizer
        TreeNode *root = program(tokens);               // creates tree

        generateCodePreorder(root, outputFile);
        printGlobalVariables(outputFile);

        outputFile.close();
    }
    return 0;
}
