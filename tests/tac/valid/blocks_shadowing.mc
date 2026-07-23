int x = 1;

{
    print x;
    int x = x + 1;
    print x;

    {
        int x = 3;
        print x;
    }

    print x;
}

print x;

{
    int x = 4;
    print x;
}
