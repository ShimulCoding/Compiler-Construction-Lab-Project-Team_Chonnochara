int x = 1;
bool choose = true;

if (choose) {
    int x = x + 1;
    print x;
} else {
    int x = 3;
    print x;
}

print x;

while (choose) {
    int x = x + 4;
    print x;
    choose = false;
}

print x;
