/*
    calculator08buggy.cpp

    Helpful comments removed.

    We have inserted 3 bugs that the compiler will catch and 3 that it won't.

    add comments
*/

#include "../includes/ppp.hpp"

//------------------------------------------------------------------------------

constexpr char number = '8';    // t.kind == number means that t is a number Token.
constexpr char quit = 'q';      // t.kind == quit means that t is a quit Token.
constexpr char print = ';';     // t.kind == print means that t is a print Token.
constexpr char* prompt = "> ";
constexpr char* result = "= ";  // used to indicate that what follows is a result

constexpr char name = 'a';      // name token
constexpr char let = 'L';       // declaration token
constexpr char* declkey = "let"; // declaration keyword

//------------------------------------------------------------------------------

struct Token {
    char kind;
    double value;
    std::string name;
    Token(char ch, double val = 0.0) :kind(ch), value(val) {}
    Token(char ch, std::string s) :kind(ch), name(s) {}
};

//------------------------------------------------------------------------------

// place to hold variable name value
struct Variable {
    std::string name;
    double value{};
};

// container for all the variables
std::vector<Variable> names;

// return the value of the Variable with the input name.
double get_value(const std::string s)
{
    for (int i = 0; i < names.size(); ++i)
        if (names[i].name == s) return names[i].value;
    ppp::error("get: undefined name ", s);
}

// set the Variable of the named to a double value.
void set_value(const std::string s, const double d)
{
    for (int i = 0; i < names.size(); ++i)
        if (names[i].name == s) {
            names[i].value = d;
            return;
        }
    ppp::error("set: undefined name ", s);
}

// is a name already declared
bool is_declared(const std::string s)
{
    for (int i = 0; i < names.size(); ++i)
        if (names[i].name == s) return true;
    return false;
}

// add name value to a vector of Variables
double define_name(const std::string s, const double d)
{
    if (is_declared(s)) ppp::error(s, " declared twice");
    names.push_back(Variable{ s, d });
    return d;
}

//------------------------------------------------------------------------------

// place to hold valid Tokens from cin
class Token_stream {
public:
    Token_stream() :full(false), buffer('\0') { }

    // get a Token to place in the stream
    Token get();

    // put a Token back
    void putback(const Token t);

    // discard characters up to and including the given input kind token
    void ignore(const char c);      

private:                            
    bool full;      // is there a Token in the buffer?
    Token buffer;   // here is where we keep a Token put back using putback()
};

Token Token_stream::get()
{
    if (full) {
        full = false;
        return buffer;
    }
    char ch;
    std::cin >> ch;
    switch (ch) {
    case print:
    case quit:
    case '(':
    case ')':
    case '+':
    case '-':
    case '*':
    case '/':
        //case '%':
    case '=':
        return Token(ch);
    case '.':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    {
        std::cin.putback(ch);
        double val;
        std::cin >> val;
        return Token(number, val);
    }
    default:
        if (isalpha(ch)) {
            std::string s;
            s += ch;
            while (std::cin.get(ch) && (isalpha(ch) || isdigit(ch))) s += ch;
            std::cin.putback(ch);
            if (s == declkey) return Token(let);
            //if (s == "quit") return Token(name);
            return Token(name, s);
        }
        ppp::error("Bad token");
    }
}

void Token_stream::putback(const Token t)
{
    if (full) {
        ppp::error("putback() into a full buffer");
    }
    buffer = t;
    full = true;
}

void Token_stream::ignore(const char c)
{
    if (full && c == buffer.kind) {
        full = false;
        return;
    }
    full = false;
    ppp::clear_cin_buffer(c);
}

Token_stream ts;

//------------------------------------------------------------------------------

double expression();

//------------------------------------------------------------------------------

double primary()
{
    Token t = ts.get();
    double d{};
    switch (t.kind) {
    case '(': {
        d = expression();
        t = ts.get();
        if (t.kind != ')') {
            ppp::error("')' expected");
        }
        return d;
    }
    case '-':
        return -1 * primary();
    case '+':
        return primary();
    case number:
        return t.value;
    case name:
        return get_value(t.name);
    default:
        ppp::error("primary expected");
    }
}

//------------------------------------------------------------------------------

double term()
{
    double left = primary();
    while (true) {
        Token t = ts.get();
        switch (t.kind) {
        case '*':
            left *= primary();
            break;
        case '/':
        {	double d = primary();
        if (d == 0) ppp::error("divide by zero");
        left /= d;
        break;
        }
        default:
            ts.putback(t);
            return left;
        }
    }
}

//------------------------------------------------------------------------------

double expression()
{
    double left = term();
    while (true) {
        Token t = ts.get();
        switch (t.kind) {
        case '+':
            left += term();
            break;
        case '-':
            left -= term();
            break;
        default:
            ts.putback(t);
            return left;
        }
    }
}

//------------------------------------------------------------------------------

double declaration()
{
    Token t = ts.get();
    if (t.kind != name) {
        ppp::error("name expected in declaration");
    }
    if (is_declared(t.name)) {
        ppp::error(t.name, " declared twice");
    }
    Token t2 = ts.get();
    if (t2.kind != '=') {
        ppp::error("= missing in declaration of ", t.name);
    }
    double d = expression();
    define_name(t.name, d);
    return d;
}

//------------------------------------------------------------------------------

double statement()
{
    Token t = ts.get();
    switch (t.kind) {
    case let:
        return declaration();
    default:
        ts.putback(t);
        return expression();
    }
}

//------------------------------------------------------------------------------

void clean_up_mess()
{
    ts.ignore(print);
}

//------------------------------------------------------------------------------

void calculate()
{
    while (true) try {
        std::cout << prompt;
        Token t = ts.get();
        while (t.kind == print) t = ts.get();
        if (t.kind == quit) return;
        ts.putback(t);
        std::cout << result << statement() << std::endl;
    }
    catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        clean_up_mess();
    }
}

//------------------------------------------------------------------------------

int main()
try {
    SET_IOSYNC;
    calculate();
    return 0;
}
catch (std::exception& e) {
    std::cerr << "exception: " << e.what() << std::endl;
    ppp::keep_window_open();
    return 1;
}
catch (...) {
    std::cerr << "exception\n";
    ppp::keep_window_open();
    return 2;
}