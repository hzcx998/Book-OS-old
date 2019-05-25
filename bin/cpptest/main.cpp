#include <iostream>
#include <algorithm>
using namespace std;
int main(int argc, char *argv0[]) {
    int a[] = {1,2,5,6};
    sort(a,a + 4);
    for (int i = 0;i < 4;i ++) {
        cout << a[i] << endl;
    }
	return 0;
}