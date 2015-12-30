#include<iostream>
#include<windows.h>
using namespace std;
typedef struct{
	DWORD PageSizeBytes;
}Param;
HANDLE allo=CreateSemaphore(NULL,1,1,NULL);
HANDLE trac=CreateSemaphore(NULL,0,1,NULL);
LPVOID addr;	//�ڴ����ҳ��ĵ��׵�ַ
MEMORY_BASIC_INFORMATION memoryInfo;	//�����ڴ�״̬��Ϣ
DWORD WINAPI Allocator(LPVOID LpParam){ //ģ���ڴ����
	Param *param=(Param *)LpParam;
	WaitForSingleObject(allo,INFINITE);
    addr=VirtualAlloc(NULL,2*param->PageSizeBytes,MEM_RESERVE,PAGE_READWRITE);//���뱣������ҳ��
	if(addr==NULL){
		GetLastError();
		cout<<"Failed in VirtualAlloc!\n";
		return 0;
	}
	ReleaseSemaphore(trac,1,NULL);
	WaitForSingleObject(allo,INFINITE);
	VirtualAlloc(addr,2*param->PageSizeBytes,MEM_COMMIT,PAGE_READWRITE);//������������ҳ���ύ
	ReleaseSemaphore(trac,1,NULL);
	WaitForSingleObject(allo,INFINITE);
	VirtualLock(addr,2*param->PageSizeBytes);//�����ύ������ҳ��
	ReleaseSemaphore(trac,1,NULL);
	WaitForSingleObject(allo,INFINITE);
	VirtualUnlock(addr,2*param->PageSizeBytes);//��������������ҳ��
	ReleaseSemaphore(trac,1,NULL);
	WaitForSingleObject(allo,INFINITE);
	VirtualFree(addr,2*param->PageSizeBytes,MEM_DECOMMIT);//�����ύ������ҳ��
	ReleaseSemaphore(trac,1,NULL);
	WaitForSingleObject(allo,INFINITE);
	VirtualFree(addr,2*param->PageSizeBytes,MEM_FREE);//�ͷ��ύ������ҳ��
	ReleaseSemaphore(trac,1,NULL);
	return 0;
} 
DWORD WINAPI Tracker(){ //�ڴ����
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