extern "C" {
	#include <stdio.h>
	#include <stdlib.h>
}
#include <vector>
using namespace std;
int main(int argc, char *argv0[]) {
	printf("test\n");
	vector<int> data;
	data.push_back(0);
	if(data.empty()) {
		printf("empty");
	}else {
		printf("not empty");
	}
	for(int i = 0;i < 10;i ++) {
		data.push_back(i);
	}
	printf("pushed \n");
	for(int b = 0;b < 1;b ++) {
		printf("%d \n",data.pop_back());
	}
	
	return 0;
}