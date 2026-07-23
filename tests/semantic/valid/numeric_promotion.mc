float result = 1 + 2.5;
int difference = 3 - 1;
float quotient = 5.0 / 2;
int remainder = 5 % 2;
bool ordered = 1 < 2.0;
bool upper = 3.0 > 2;
bool lower_or_equal = 2 <= 2.0;
bool upper_or_equal = 3.0 >= 3;
bool equal = result == 3.5;
bool mixed_equal = 1 == 1.0;
bool different = remainder != difference;
bool logic = !false && ordered && upper && lower_or_equal
    && upper_or_equal || equal || different;
result = quotient + 1.0;
print result;
print logic;
print mixed_equal;
