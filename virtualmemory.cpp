#include<iostream>
#include<windows.h>
using namespace std;
typedef struct{
	DWORD PageSizeBytes;
}Param;
HANDLE allo=CreateSemaphore(NULL,1,1,NULL);
HANDLE trac=CreateSemaphore(NULL,0,1,NULL);
LPVOID addr;	//内存操作页面的的首地址
MEMORY_BASIC_INFORMATION memoryInfo;	//保存内存状态信息
DWORD WINAPI Allocator(LPVOID LpParam){ //模拟内存分配活动
	Param *param=(Param *)LpParam;
	WaitForSingleObject(allo,INFINITE);
    addr=VirtualAlloc(NULL,2*param->PageSizeBytes,MEM_RESERVE,PAGE_READWRITE);//申请保留两个页面
	if(addr==NULL){
		GetLastError();
		cout<<"Failed in VirtualAlloc!\n";
		return 0;
	}
	ReleaseSemaphore(trac,1,NULL);
	WaitForSingleObject(allo,INFINITE);
	VirtualAlloc(addr,2*param->PageSizeBytes,MEM_COMMIT,PAGE_READWRITE);//将保留的两个页面提交
	ReleaseSemaphore(trac,1,NULL);
	WaitForSingleObject(allo,INFINITE);
	VirtualLock(addr,2*param->PageSizeBytes);//锁定提交的两个页面
	ReleaseSemaphore(trac,1,NULL);
	WaitForSingleObject(allo,INFINITE);
	VirtualUnlock(addr,2*param->PageSizeBytes);//解锁锁定的两个页面
	ReleaseSemaphore(trac,1,NULL);
	WaitForSingleObject(allo,INFINITE);
	VirtualFree(addr,2*param->PageSizeBytes,MEM_DECOMMIT);//回收提交的两个页面
	ReleaseSemaphore(trac,1,NULL);
	WaitForSingleObject(allo,INFINITE);
	VirtualFree(addr,2*param->PageSizeBytes,MEM_FREE);//释放提交的两个页面
	ReleaseSemaphore(trac,1,NULL);
	return 0;
} 
DWORD WINAPI Tracker(){ //内存跟踪
	for(int i=0;i<6;i++){
		WaitForSingleObject(trac,INFINITE);
		VirtualQuery(addr,&memoryInfo,sizeof(memoryInfo));
		cout<<"BaseAddress:"<<memoryInfo.BaseAddress<<endl<<"AllocationBase:"<<memoryInfo.AllocationBase<<endl;
		cout<<"AllocationProtect:"<<memoryInfo.AllocationProtect<<endl<<"RegionSize:"<<memoryInfo.RegionSize<<endl;
		cout<<"State:"<<memoryInfo.State<<endl<<"Protect:"<<memoryInfo.Protect<<endl<<"Type:"<<memoryInfo.Type<<endl<<endl;
		ReleaseSemaphore(allo,1,NULL);
	}
	return 0;
}
int main(){
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	Param param;
	param.PageSizeBytes=sysinfo.dwPageSize;
	return 0;
}