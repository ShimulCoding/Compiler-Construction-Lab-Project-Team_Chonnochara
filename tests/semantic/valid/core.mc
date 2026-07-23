int x = 1;
int pending;
float total = x + 2.5;
bool ready = total >= 3.5;
pending = 3;
x = x + 1;
print pending;
print x;
if (ready) {
    int inner = x * 2;
    print inner;
} else {
    print total;
}
while (ready && x < 4) {
    x = x + 1;
    ready = x < 4;
}
