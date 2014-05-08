#include <stdio.h>
#include <locale.h>

int main()
{
    int bignum=12345678;

    setlocale(LC_ALL,"");

    printf("Big number: %'d\n",bignum);

    return 0;
}
