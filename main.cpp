#include <iostream>
#include "Function.h"

using namespace std;

string many_a(int a) {
    string res;
    for (int i = 0; i < a; i++) {
        res += 'a';
    }

    return res;
}

void test_function() {
    Function<string(int)> function(many_a);

    assert(function(3) == "aaa");
}

void test_function_assignment() {
    Function<string(int)> function = many_a;

    assert(function(4) == "aaaa");
}

void test_lambda() {
    Function<int()> function([](){ return 1;});

    assert(function() == 1);
}

void test_lambda_byref() {
    int a = 1;
    Function<int()> function([&](){ return a++;});

    assert(function() == 1);
    assert(function() == 2);
    assert(function() == 3);
    assert(a == 4);
}

void test_lambda_byval() {
    int a = 1;
    Function<int()> function([=](){ return a;});
    a++;

    assert(function() == 1);
    assert(a == 2);
}

void test_lambda_assignment() {
    Function<int()> function = [](){ return 2;};

    assert(function() == 2);
}

void test_empty() {
    Function<void()> a;

    assert(!a);
}

void test_copy_empty() {
    Function<void()> a([](){int t = 1;});

    assert(a);

    Function<void()> b;
    assert(!b);
    a = b;

    assert(!a);
}

void test_swap() {
    Function<int()> a([](){return 1;});
    Function<int()> b([](){return 2;});

    a.swap(b);

    assert(a() == 2);
    assert(b() == 1);
}

void test_move() {
    Function<int()> a([](){return 1;});
    Function<int()> b(move(a));

    assert(b() == 1);
}

void test_move_assignment() {
    Function<int()> a([](){return 1;});
    Function<int()> b([](){return 2;});

    a = move(b);
    assert(a() == 2);
}

int main() {
    test_function();
    test_function_assignment();

    test_lambda();
    test_lambda_byref();
    test_lambda_byval();
    test_lambda_assignment();

    test_empty();
    test_copy_empty();
    test_swap();

    test_move();
    test_move_assignment();

    cout << "TESTS OK";
    return 0;
}