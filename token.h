#pragma once

enum Token {
    Main = 0, Const, Int, Void,
    
    While, Break, 
    
    If, Else, Continue,

    Not, And, Or, 

    GetInt, Printf,

    Return,

    Plus, Minus, Mult, Div, Mod, 

    Less, Greater, GreaterEq, LessEq, Equal, NotEq, 

    Assign, SemiCon, Comma,
    
    LeftParent, RightParent,

    LeftBrack, RightBrack,

    LeftBrace, RightBrace,

    String, Integer, Identifier,

    _TokenCount
};

struct CString {
    char* literal;
    size_t len;
};


struct ASTNode
{
    enum Token type;
    struct CString literal;
    union NodeData
    {
        
    } data;
};
