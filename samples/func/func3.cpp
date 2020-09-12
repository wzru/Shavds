int a();
int b();
int c();
int d();
int e();

int a()
{
    return b();
}

int b()
{
    return c();
}

int c()
{
    return d() * d();
}

int d()
{
    return e() + e();
}

int e()
{
    return 66666;
}