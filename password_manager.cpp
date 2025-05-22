#ifndef _PASSWORD_MANAGER_CPP_
#define _PASSWORD_MANAGER_CPP_

#include <cmath>
#include <tuple>

namespace {
    #define PASSWORD_LENGTH 4
}

class PasswordManager {
private:
    int password = 1218;
    int passwordBuffer = 0;
    char cursor = 1;

    std::tuple<int, int, int, int> decompose() {
        // 4자리일 경우, 1일때 1000, 2일때 100, 3일때 10, 4일때 1 
        int unit = pow(10, PASSWORD_LENGTH - cursor);
        int prefix = (int)(passwordBuffer / (unit * 10)) * (unit * 10) ;
        int postfix = passwordBuffer % unit;

        int select = ((int)(passwordBuffer / unit) % 10) * unit;
        return std::make_tuple(prefix, select, postfix, unit);
    }
public:
    bool authorization() {
        bool result = false;
        if (password == passwordBuffer) {
            result = true;
        }

        return result;
    }

    void changePassword() {
        password = passwordBuffer;
        passwordBuffer = 0;
    }

    char cursorLeft() {
        cursor--;
        if (cursor < 1) cursor = 1;
        return cursor;
    }

    char cursorRight() {
        cursor++;
        if (cursor > PASSWORD_LENGTH) cursor = PASSWORD_LENGTH;
        return cursor;
    }

    char getCursor() {
        return cursor;
    }

    char resetCursor() {
        cursor = 1;
        return cursor;
    }

    int inputPlus() {
        auto [prefix, select, postfix, unit] = decompose(); 
        select = (select + unit) % (unit * 10);

        passwordBuffer = prefix + select + postfix;
        return passwordBuffer;
    }

    int inputMinus() {
        auto [prefix, select, postfix, unit] = decompose(); 
        select = select - unit;
        if (select < 0) select = unit * 9;

        passwordBuffer = prefix + select + postfix;
        return passwordBuffer;
    }

    int getInput() {
        return passwordBuffer;
    }

    int resetInput() {
        passwordBuffer = 0;
        return passwordBuffer;
    }
};

#endif