int fib2(int n)
{
    if (n < 2)
        return 1;
    else {
        int a = 1, b = 1, c;
        for (int i = 2; i <= n; ++i) c = a + b, a = b, b = c;
        return c;
    }
}