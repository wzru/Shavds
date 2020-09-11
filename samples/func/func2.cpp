int aa();
int bb();
int cc();
int dd();
int ee();

int aa()
{
    return bb() / cc();
}

int bb()
{
    return cc() & dd();
}

int cc()
{
    return dd() | dd() - 1;
}

int dd()
{
    return ee() + ee() + 3;
}

int ee()
{
    return 23333;
}