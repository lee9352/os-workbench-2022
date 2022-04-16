#include "thread.h"
void Ta() { while (1) { printf("a"); } }
void Tb() { while (1) { printf("b"); } }
int main(){
	create(Ta);
	create(Tb);
	return 0;
}
//用Top查看该程序cpu占用率，超过了100%