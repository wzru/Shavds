double GPA1(int score)
{
    if (score < 60)
        return 0;
    else if (score < 85)
        return (score - 60) / 10.0 + 1.5;
    else
        return 4.0;
}