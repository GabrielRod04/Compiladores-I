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
    OP_SLASH,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_LT,
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

private:
    const std::string src;
    size_t pos;
    int linea, columna;

    // --- utilidades de bajo nivel ---

    char actual() const
    {
        return pos < src.size() ? src[pos] : '\0';
    }

    char siguiente() const
    {
        return (pos + 1) < src.size() ? src[pos + 1] : '\0';
    }

    void avanzar()
    {
        if (actual() == '\n')
        {
            linea++;
            columna = 1;
        }
        else
        {
            columna++;
        }
        pos++;
    }

    void saltarEspaciosYComentarios()
    {
        while (true)
        {
            char c = actual();
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            {
                avanzar();
            }
            else if (c == '/' && siguiente() == '/')
            {
                // comentario de línea: ignorar hasta '\n' o EOF
                while (actual() != '\n' && actual() != '\0')
                    avanzar();
            }
            else
            {
                break;
            }
        }
    }

    // reconocedores por categoría (cada uno = un AFD)

    Token reconocerIdentificadorOReservada(int lin, int col)
    {
        std::string lexema;
        while (std::isalnum((unsigned char)actual()) || actual() == '_')
        {
            lexema += actual();
            avanzar();
        }
        auto it = Palabras_Reservadas.find(lexema);
        TokenType tipo = (it != Palabras_Reservadas.end()) ? it->second : TokenType::ID;
        return {tipo, lexema, lin, col};
    }

    Token reconocerNumero(int lin, int col)
    {
        std::string lexema;
        bool esFloat = false;

        while (std::isdigit((unsigned char)actual()))
        {
            lexema += actual();
            avanzar();
        }
        // Parte decimal: solo si hay un '.' seguido de un dígito
        // (evita confundir "10.suma()" con un número mal formado)
        if (actual() == '.' && std::isdigit((unsigned char)siguiente()))
        {
            esFloat = true;
            lexema += actual();
            avanzar();
            while (std::isdigit((unsigned char)actual()))
            {
                lexema += actual();
                avanzar();
            }
        }
        TokenType tipo = esFloat ? TokenType::NUM_FLOAT : TokenType::NUM_INT;
        return {tipo, lexema, lin, col};
    }

    Token reconocerCadena(int lin, int col)
    {
        std::string lexema;
        avanzar(); // consume la comilla inicial '"'
        while (actual() != '"')
        {
            if (actual() == '\0' || actual() == '\n')
            {
                throw LexerError("Cadena de texto sin cerrar", lin, col);
            }
            lexema += actual();
            avanzar();
        }
        avanzar(); // consume la comilla final '"'
        return {TokenType::STRING_LIT, lexema, lin, col};
    }

    Token reconocerCaracter(int lin, int col)
    {
        avanzar(); // consume la comilla inicial '\''
        std::string lexema;
        if (actual() == '\0')
            throw LexerError("Literal char sin cerrar", lin, col);
        lexema += actual();
        avanzar();
        if (actual() != '\'')
            throw LexerError("Literal char mal formado", lin, col);
        avanzar(); // consume la comilla final '\''
        return {TokenType::CHAR_LIT, lexema, lin, col};
    }

    // Operadores y delimitadores: se revisan primero los de 2 caracteres
    Token reconocerSimbolo(int lin, int col)
    {
        char c = actual();
        char c2 = siguiente();

        // --- de dos caracteres ---
        if (c == '-' && c2 == '>')
        {
            avanzar();
            avanzar();
            return {TokenType::OP_ARROW, "->", lin, col};
        }
        if (c == '=' && c2 == '=')
        {
            avanzar();
            avanzar();
            return {TokenType::OP_EQ, "==", lin, col};
        }
        if (c == '!' && c2 == '=')
        {
            avanzar();
            avanzar();
            return {TokenType::OP_NEQ, "!=", lin, col};
        }
        if (c == '<' && c2 == '=')
        {
            avanzar();
            avanzar();
            return {TokenType::OP_LE, "<=", lin, col};
        }
        if (c == '>' && c2 == '=')
        {
            avanzar();
            avanzar();
            return {TokenType::OP_GE, ">=", lin, col};
        }
        if (c == '&' && c2 == '&')
        {
            avanzar();
            avanzar();
            return {TokenType::OP_AND, "&&", lin, col};
        }
        if (c == '|' && c2 == '|')
        {
            avanzar();
            avanzar();
            return {TokenType::OP_OR, "||", lin, col};
        }

        //  de un caracter
        switch (c)
        {
        case '+':
            avanzar();
            return {TokenType::OP_PLUS, "+", lin, col};
        case '-':
            avanzar();
            return {TokenType::OP_MINUS, "-", lin, col};
        case '*':
            avanzar();
            return {TokenType::OP_STAR, "*", lin, col};
        case '/':
            avanzar();
            return {TokenType::OP_SLASH, "/", lin, col};
        case '<':
            avanzar();
            return {TokenType::OP_LT, "<", lin, col};
        case '>':
            avanzar();
            return {TokenType::OP_GT, ">", lin, col};
        case '!':
            avanzar();
            return {TokenType::OP_NOT, "!", lin, col};
        case '=':
            avanzar();
            return {TokenType::OP_ASSIGN, "=", lin, col};
        case '{':
            avanzar();
            return {TokenType::LBRACE, "{", lin, col};
        case '}':
            avanzar();
            return {TokenType::RBRACE, "}", lin, col};
        case '(':
            avanzar();
            return {TokenType::LPAREN, "(", lin, col};
        case ')':
            avanzar();
            return {TokenType::RPAREN, ")", lin, col};
        case ',':
            avanzar();
            return {TokenType::COMMA, ",", lin, col};
        case ':':
            avanzar();
            return {TokenType::COLON, ":", lin, col};
        case ';':
            avanzar();
            return {TokenType::SEMI, ";", lin, col};
        }

        // Ningún AFD lo reconoció -> caracter inválido
        std::string desconocido(1, c);
        avanzar();
        return {TokenType::UNKNOWN, desconocido, lin, col};
    }

    // función principal: decide qué reconocedor invocar
    Token siguienteToken()
    {
        saltarEspaciosYComentarios();

        int lin = linea, col = columna;
        char c = actual();

        if (c == '\0')
            return {TokenType::END_OF_FILE, "", lin, col};

        if (std::isalpha((unsigned char)c) || c == '_')
            return reconocerIdentificadorOReservada(lin, col);
        if (std::isdigit((unsigned char)c))
            return reconocerNumero(lin, col);
        if (c == '"')
            return reconocerCadena(lin, col);
        if (c == '\'')
            return reconocerCaracter(lin, col);

        return reconocerSimbolo(lin, col);
    }
};

#endif
