#include <stdio.h>

void test(char a[7], char *b) {

    sprintf(a ,"%s", b);

}

int main(int argc, char const *argv[])
{
    char a[7];
    char b[7];

    test(a, "aaaa");

    printf("%s\n", a);

       test(b, "bbb");

    printf("%s\n", b);


    test(a, "aaaa");

    printf("%s\n", a);

       test(b, "bbb");

    printf("%s\n", b);


        test(a, "aaaa");

    printf("%s\n", a);

       test(b, "bbb");

    printf("%s\n", b);



    /* code */
    return 0;
}
