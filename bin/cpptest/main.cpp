extern "C" {
	#include <stdio.h>
	#include <stdlib.h>
}
#include "vector.h"
int main(int argc, char *argv0[]) {
	vector<int> data;
	data.push_back(0);
	for(int i = 0;i < 10;i ++) {
		data.push_back(i);
	}
	for(int i = 0;i < 11;i ++) {
		printf("%d",data.pop_back());
	}
	return 0;
}