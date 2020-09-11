int sum3(int n)
{
    int s = 0;
    for (int i = 1; i <= n; ++i) s += i;
    int nouse;
    for (int i = 1; i <= 100000; ++i) { nouse *= i; }
    return s;
}