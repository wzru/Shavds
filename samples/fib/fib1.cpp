int fib1(int n)
{
    if (n < 2)
        return 1;
    else
        return fib1(n - 1) + fib1(n - 2);
}

int main()
{
    if (fib1(2) > fib1(4))
        return fib1(10);
    else if (fib1(3) > fib1(2))
        return fib1(20);
    else
        return fib1(30);
    return 0;
}