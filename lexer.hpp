#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <stdexcept>

using std::runtime_error;
using std::string;
using std::unordered_map;
using std::vector;

// Categorias de Tokens

enum class TokenType
{
    // Palabras reservadas con prefijo KW(Key word)
    KW_FN,
    KW_LET,
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_FOR,
    KW_IN,
    KW_RETURN,
    KW_TRUE,
    KW_FALSE,

    // Tipos Primitivos
    TYPE_I32,
    TYPE_F64,
    TYPE_BOOL,
    TYPE_CHAR,
    TYPE_STR,

    // Identificador y literales
    ID,
    NUM_INT,
    NUM_FLOAT,
    STRING_LIT,
    CHAR_LIT,

    // Operadores
    OP_PLUS,
    OP_MINUS,
    OP_STAR,
    OP_SPLASH,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_LIT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_EQ,
    OP_NEQ,
    OP_ASSIGN,
    OP_ARROW,

    // Delimitadores
    LBRACE,
    RBRACE,
    LPAREN,
    RPAREN,
    COMMA,
    COLON,
    SEMI,

    END_OF_FILE,
    UNKNOWN
};

struct Token
{
    TokenType type;
    string lexema;
    int linea;
    int columna;
};

static const unordered_map<string, TokenType> Palabras_Reservadas = {
    {"fn", TokenType::KW_FN},
    {"let", TokenType::KW_LET},
    {"if", TokenType::KW_IF},
    {"else", TokenType::KW_ELSE},
    {"while", TokenType::KW_WHILE},
    {"for", TokenType::KW_FOR},
    {"in", TokenType::KW_IN},
    {"return", TokenType::KW_RETURN},
    {"true", TokenType::KW_TRUE},
    {"false", TokenType::KW_FALSE},
    {"i32", TokenType::TYPE_I32},
    {"f64", TokenType::TYPE_F64},
    {"bool", TokenType::TYPE_BOOL},
    {"char", TokenType::TYPE_CHAR},
    {"str", TokenType::TYPE_STR},
};

class LexerError : public runtime_error
{
public:
    LexerError(const string &msg, int linea, int columna) : runtime_error(msg), linea(linea), columna(columna) {}
    int linea, columna;
};

class Lexer
{
public:
    explicit Lexer(const string &fuente) : src(fuente), pos(0), linea(1), columna(1) {}

    // Recorre todo el codigo fuente y devuelve la lista completa de Tokens
    vector<Token> tokenizar()
    {
        vector<Token> tokens;
        while (true)
        {
            Token t = siguienteToken();
            tokens.push_back(t);
            if (t.type == TokenType::END_OF_FILE)
                break;
        }
        return tokens;
    }
};

#endif
