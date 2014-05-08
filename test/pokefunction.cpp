#include <iostream>

using namespace std;

void func1()
{ cout << "Hello from func 1\n"; }
void func2()
{ cout << "Hello from func 2\n"; }

int main()
{
    void (*fp1)()=&func1;
    void (*fp2)()=&func2;
    cout << "fp1 = 0x" << hex << (long)fp1 << dec << "\n";
    cout << "fp2 = 0x" << hex << (long)fp2 << dec << "\n";
    cout << "fp1==fp2: " << (fp1==fp2) << "\n";
    fp1();
    fp2();
    return 0;
}
