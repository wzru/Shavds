int a();
int b();
int c();
int d();
int e();

int a()
{
    return b() * c();
}

int b()
{
    return c() + d();
}

int c()
{
    return d() ^ d() - 1;
}

int d()
{
    return e() * e();
}

int e()
{
    return 114514;
}

int main()
{
    return a()+e();
}