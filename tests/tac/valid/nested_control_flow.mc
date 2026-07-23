int x = 2;
bool run = true;

while (run && x > 0) {
    if (x == 1) {
        print x;
    } else {
        while (run) {
            run = false;
        }
    }

    x = x - 1;
}
