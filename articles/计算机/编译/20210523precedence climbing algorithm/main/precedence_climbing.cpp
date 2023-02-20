#include <iostream>
#include <queue>
#include <optional>

using namespace std;

string TokenKindString[] = {
    "PAR_L", "PAR_R",
    "NUM", "OP_ADD", "OP_SUB",
    "OP_MUL", "OP_DIV", "END"
};

enum TokenKind {
  PAR_L,
  PAR_R,
  NUM,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  END
};

struct Token {
  TokenKind kind;
  int value; // for Token NUM, hold its value.
  explicit Token(TokenKind tk) : kind(tk) {
  }

  explicit Token(int v) : kind(NUM), value(v) {
  }

  Token() = default;

  string toString() const {
    if (kind == NUM) {
      return TokenKindString[kind] + '(' + to_string(value) + ')';
    }
    return TokenKindString[kind];
  }

  bool isEND() const {
    return kind == END;
  }

  bool isOP() const {
    return kind == OP_ADD || kind == OP_SUB
           || kind == OP_MUL || kind == OP_DIV;
  }
};

struct Lexer {
  string s;
  int pos = 0;
  queue<Token> buf;

  void setString(string str) {
    s = std::move(str);
    pos = 0;
  }

  inline bool isop(char c) {
    return c == '+' || c == '-'
           || c == '*' || c == '/';
  }

  int parsingNumber() {
    int end = pos;
    while (end < s.size() && isdigit(s[end])) end++;
    int value = stoi(s.substr(pos, (end - pos)));
    pos = end;
    return value;
  }

  // assume all input will be valid.
  optional<Token> tokenize() {
    while (pos < s.size() && s[pos] == ' ') pos++;
    if (pos == s.size()) return nullopt;
    char c = s[pos];
    if (isdigit(c)) {
      int value = parsingNumber();
      return make_optional<Token>(value);
    } else if (isop(s[pos])) {
      pos++;
      switch (c) {
        case '+':
          return make_optional<Token>(OP_ADD);
        case '-':
          return make_optional<Token>(OP_SUB);
        case '*':
          return make_optional<Token>(OP_MUL);
        case '/':
          return make_optional<Token>(OP_DIV);
        default:
          return nullopt;
      }
    } else if (c == '(') {
      pos++;
      return make_optional<Token>(PAR_L);
    } else if (c == ')') {
      pos++;
      return make_optional<Token>(PAR_R);
    } else {
      return nullopt;
    }
  }

  void fillBuf() {
    if (!buf.empty()) return;
    if (pos == s.size()) {
      buf.emplace(END);
      return;
    }
    optional<Token> tokenOpt = tokenize();
    buf.push(tokenOpt.value_or(Token{END}));
  }

  bool testFront(TokenKind expect) {
    fillBuf();
    return buf.front().kind == expect;
  }

  Token nextToken() {
    fillBuf();
    Token t = buf.front();
    buf.pop();
    return t;
  }

  Token &peek() {
    fillBuf();
    return buf.front();
  }
};

struct Calculator {
  Lexer lexer;
  string s;

  static int getPrecedence(Token op) {
    switch (op.kind) {
      case OP_ADD:
      case OP_SUB:
        return 1;
      case OP_MUL:
      case OP_DIV:
        return 2;
      default:
        throw runtime_error("bad operator.");
    }
  }

  static int combine(int l, Token op, int r) {
    switch (op.kind) {
      case OP_ADD:
        return l + r;
      case OP_SUB:
        return l - r;
      case OP_MUL:
        return l * r;
      case OP_DIV:
        return l / r;
      default:
        throw runtime_error("bad operator.");
    }
  }

  int primary() {
    // primary -> NUM | '(' expr ')'
    if (lexer.testFront(NUM)) {
      Token t = lexer.nextToken();
      return t.value;
    } else if (lexer.testFront(PAR_L)) {
      lexer.nextToken();
      int v = expr(0);
      if (!lexer.testFront(PAR_R)) {
        throw runtime_error("unexpected token for primary.");
      }
      lexer.nextToken();
      return v;
    }
    throw runtime_error("unexpected token for primary.");
  }

  int singleExpr(int limits) {
    // singleExpr -> '-' singleExpr | primary
    if (lexer.testFront(OP_SUB)) {
      lexer.nextToken();
      return -singleExpr(limits);
    } else {
      return primary();
    }
  }

  int expr(int limits) {
    // expr -> singleExpr {OP expr}
    int lvalue = singleExpr(limits);
    Token op;
    while (lexer.peek().isOP() && getPrecedence(lexer.peek()) > limits) {
      op = lexer.nextToken();
      int rvalue = expr(getPrecedence(op));
      lvalue = combine(lvalue, op, rvalue);
    }
    return lvalue;
  }

  int calculate(string str) {
    lexer.setString(std::move(str));
    return expr(0);
  }
};


int main(int argc, const char *argv[]) {
  Calculator cal;
  cout << cal.calculate(argv[1]) << endl;
  return 0;
}
