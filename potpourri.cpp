// potpourri.cpp - Learn advanced language features
//   1. `try-block` for execption handling
//   2. Constant template
//   3. `unique_ptr`, `make_unique`, `shared_ptr` and `make_shared`
//   4. Lambda expressions

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <functional>

template <typename T, size_t N>
struct Vec {
    T data[N];

    // A default constructor
    Vec() : data() {}

    Vec(std::initializer_list<T> il) {
        if (il.size() != N)  {
            throw std::invalid_argument("Wrong number of arguments!");
        }

        std::copy(il.begin(), il.end(), data);
    }

    // Copy constructor
    Vec(const Vec<T,N>& v)
    {
        std::copy(&v[0], &v[N], data);
    }

    // Member access 
    T& operator[](size_t idx) { return data[idx]; }
    const T& operator[](size_t idx) const { return data[idx]; }

    // Add operator
    template <typename U, size_t M>
    Vec<U, M> operator+(const Vec<U, M>& v) {
        if (N != M)
            throw std::invalid_argument("Vector size mismatch!");

        Vec<U, M> result;
        for (int i = 0; i < M; i++)
            result[i] = data[i] + v[i];

        return result;
    }

    // Dot product operator
    template <typename U, size_t M>
    Vec<U, M> operator*(const Vec<U, M>& v) {
        if (N != M)
            throw std::invalid_argument("Vector size mismatch!");

        Vec<U, M> result;
        for (int i = 0; i < M; i++)
            result[i] = data[i] * v[i];

        return result;
    }

    friend std::ostream& operator<<(std::ostream& os, const Vec<T, N>& v) {
        os << '[';

        for (int i = 0; i < N - 1; i++) {
            os << v.data[i] << ", ";
        }

        os << v.data[N - 1] << ']';
    }
};

struct A
{
    virtual void foo() = 0;
    ~A() { std::cout << "~A()\n"; }
    friend std::ostream& operator<<(std::ostream& os, const A& a)
    {
        os << '(' << &a << ')';
        return os;
    }
};

struct B : public A
{
    int x;
    B(int x) : x(x) {}
    ~B() { std::cout << "~B()\n"; }
    void foo() override
    { 
        std::cout << "B.foo()\n";
    }
};
struct C : public A {
    int y;
    C(int x) : y(y) {}
    ~C() { std::cout << "~C()\n"; }
    void foo() override
    { 
        std::cout << "C.foo()\n";
    }
};

void takes_ownership(std::unique_ptr<A> u_ptr)
{
    std::cout << "Took ownership of: " << *u_ptr << '\n';
}

void shared_ownership2(std::shared_ptr<A> s_ptr)
{
    std::cout << "Took ownership of: " << *s_ptr << '\n';
    std::cout << "use_count: " << s_ptr.use_count() << '\n';
}

void shared_ownership(std::shared_ptr<A> s_ptr)
{
    std::cout << "Took ownership of: " << *s_ptr << '\n';
    std::cout << "use_count: " << s_ptr.use_count() << '\n';
    shared_ownership2(s_ptr);
    std::cout << "use_count: " << s_ptr.use_count() << '\n';
}


template <typename T>
struct Adder {
    T y;

    Adder() = delete;
    Adder(T y) : y(y) {}

    T operator()(const T& t) { return y + t; }
};

int main()
{
    /**
     * 1. try-block for exception handling
     */
    std::string s;
    std::cin >> s;

    try {
        if (s.length() > 10) {
            throw std::out_of_range("Out of range");
        } else if (s.length() < 5) {
            throw std::invalid_argument("invalid argument");
        } else {
            std::cout << s << '\n';
        }
    } catch (const std::out_of_range& e) {
        std::cout << e.what() << '\n';
    } catch (const std::invalid_argument& e) {
        std::cout << e.what() << '\n';
    }

    /**
     * 2. Constant template
     */
    Vec<float, 3> f{0.1, 0.2, 0.3};
    Vec<float, 3> g{0.1, 0.2, 0.3};
    Vec<float, 4> h{0.1, 0.2, 0.3, 0.5};

    std::cout << (f * g) + f << '\n';

    try {
        // Vector size mismatch
        auto m = g + h;
        
        // Never reached
        std::cout << m << '\n';
    } catch (const std::exception& e) {
        std::cout << e.what() << '\n';
    }

    // unique_ptr for strict ownership management
    std::vector<std::unique_ptr<A>> vua;
    vua.push_back(std::make_unique<B>(3));
    vua.push_back(std::make_unique<B>(4));
    vua.push_back(std::make_unique<C>(5));
    vua.push_back(std::make_unique<C>(6));

    std::vector<A*> vpa = {
        new B(1),
        new B(2),
        new C(7),
        new C(8)
    };

    for (auto&& ap : vua) {
        ap->foo();
    }

    for (auto ap : vpa) {
        ap->foo();
    }

    // The ownership is passed to the function. Note `std::move()` used here
    takes_ownership(std::move(vua[1]));
    // `unique_ptr` can also be shared over multiple functions.
    // Note that `.release()` is called to release a shared_ptr from the `unique_ptr`.
    shared_ownership(std::shared_ptr<A>(vua[2].release()));

    for (auto&& ap : vua) {
        if (!ap) {
            std::cout << "The ownership has moved away!\n";
        } else {
            ap->foo();
        }
    }
    
    /**
     * 4. Lambda
     */
    std::string greet = "Hello, ";
    int secret = 42;

    // Captures the _reference_  of `greet`
    auto foo = [&greet](std::string name){ return greet + name; };

    // Capatures the _value_ of `secret`
    auto bar = [secret](std::string name){
        std::ostringstream oss;

        oss << name << "got the secret (" << secret << ')';

        return oss.str();
    };

    auto baz = Adder<std::string>{"Bye, "};

    std::vector<std::function<std::string(std::string)>> fv;
    fv.push_back(foo);
    fv.push_back(bar);

    // A function class is also considered as a function type
    fv.push_back(baz);

    std::string john = "John Doe";
    std::vector<std::string> sv;
    for (auto f : fv) {
        sv.push_back(f(john));
        std::cout << "a\n";
    }

    for (auto s : sv) {
        std::cout << s << '\n';
    }
}