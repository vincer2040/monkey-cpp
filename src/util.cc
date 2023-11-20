
bool is_letter(char ch) {
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || (ch == '_');
}

bool is_digit(char ch) { return '0' <= ch && ch <= '9'; }
