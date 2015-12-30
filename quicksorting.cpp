#include<iostream>
#include<fstream>
#include<windows.h>
#include<time.h>
#include<stdlib.h>
using namespace std;
#define datanum 1000000	//数据个数
#define period 10000		//生成随机数时重置种子的周期
#define InsertBorder 1000	//快排转直接插入排序的边界
//父进程对待排序列进行快排划分，划分好的两段序列交给子进程
//子序列长度小于1000时，处理线程进行直接插入排序
//采用共享内存进行通信
//进程间的同步：上一级划分的进程划分完毕，下一级划分的进程才能进行划分处理
int *data;	//待处理数组
void CreateData(){		//生成待处理数据文件data_origin,data_proc为其副本
	ofstream f0("data_origin.txt",ios_base::out);
	ofstream fdat0("data_proc.dat",ios_base::binary);
	for(int i=0;i<datanum;i++){
		if(i%period==0)
			srand((unsigned int)clock());
		int k=rand();
		f0<<k<<'\n';
		fdat0.write(reinterpret_cast<char *>(&k),sizeof(k));
	}
	f0.close();
	fdat0.close();
}
void Exchange(int &i,int &j){			//交换两个元素的位置
	int k=j;
	j=i;
	i=k;
}
void InsertSort(int *a,int left,int right){		//直接插入排序
	int i,j,tag;
	for(i=right;i>left;i--){	//将最小的元素置于首位
		if(a[i-1]>a[i])
			Exchange(a[i-1],a[i]);
	}
	for(i=left+2;i<=right;i++){
		j=i;
		tag=a[i];	//保存待插入的元素
		while(tag<a[j-1]){	//元素后移
			a[j]=a[j-1];
			j--;
		}
		a[j]=tag;	//插入位置
	}
}
int Partition(int a[],int left,int right){		//快排划分操作
	int s[3]={a[left],a[right],a[(left+right)/2]}; //选择中值为划分元素
	InsertSort(s,0,2);
	if(a[left]==s[1])
		Exchange(a[left],a[right]);
	else if(a[(left+right)/2]==s[1])
		Exchange(a[(left+right)/2],a[right]);
	int i=left-1,j=right;	//左右指针
	int e=a[right];			//划分元素
	while(true){
		while(a[++i]<=e) 
			if(i==right) break;
		while(a[--j]>=e)
			if(j==left) break;
		if(i>=j) break;
		Exchange(a[i],a[j]);
	}
	Exchange(a[i],a[right]);
	return i;	//返回划分位置
}
typedef struct{
//	int *a;
	int left;
	int right;
	int pos;
}DATA;
//int data[datanum];
HANDLE *hmutex;	//访问共享内存的互斥量，用于进程之间的同步
HANDLE threadnum=CreateSemaphore(NULL,20,INFINITE,"ThreadNum");	//限制线程数量的信号量
DATA *PDATA;
DATA _DATA[2];
DWORD WINAPI QuickSort(LPVOID lpparam){
//	WaitForSingleObject(hmutex[threadnum],INFINITE);
	WaitForSingleObject(threadnum,INFINITE);
	PDATA=(DATA *)lpparam;
	PDATA->pos=-1;		//pos=-1表示子序列长度已经小于1000
	if(PDATA->right-PDATA->left>=InsertBorder)
		PDATA->pos=Partition(data,PDATA->left,PDATA->right);
	else 
		InsertSort(data,PDATA->left,PDATA->right);
//	ReleaseMutex(hmutex[threadnum+1]);
	if(PDATA->pos!=-1){
		DATA _DATA[2];
		_DATA[0]=*PDATA;
		_DATA[1]=*PDATA;
		_DATA[0].right=PDATA->pos-1;
		_DATA[1].left=PDATA->pos+1;
		HANDLE downthread[2];//其他级划分进程句柄
		DWORD downthread0ID,downthread1ID;
		ReleaseSemaphore(threadnum,1,NULL);	
		downthread[0]=CreateThread(NULL,0,QuickSort,&_DATA[0],0,&downthread0ID);
		WaitForSingleObject(downthread[0],INFINITE);
		CloseHandle(downthread[0]);
		downthread[1]=CreateThread(NULL,0,QuickSort,&_DATA[1],0,&downthread1ID);
		WaitForSingleObject(downthread[1],INFINITE);
		CloseHandle(downthread[1]);
	}
	else ReleaseSemaphore(threadnum,1,NULL);	 
	return 0;
}
int main(){
	DATA _DATA;
//	_DATA.a=(int *)addspace;
	_DATA.left=0;
	_DATA.pos=0;
	_DATA.right=datanum-1;
//	hmutex[0]=CreateMutex(NULL,FALSE,NULL);
	HANDLE topthread;	//第一级划分线程句柄
	DWORD topthreadID;
	CreateData();
	LPCSTR filename=TEXT("data_proc.dat");
	HANDLE f=CreateFile(filename,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(f==INVALID_HANDLE_VALUE){
		cout<<"CreateFile Error!\n";
		return 0;
	}
	HANDLE filemapping=CreateFileMapping(f,NULL,PAGE_READWRITE,0,0,NULL);	 
	if(filemapping==INVALID_HANDLE_VALUE){
		cout<<"CreateFileMapping Error!\n";
		return 0;
	}
	HANDLE filemap=MapViewOfFile(filemapping,FILE_MAP_ALL_ACCESS,0,0,0);
	data=(int *)filemap;
	topthread=CreateThread(NULL,0,QuickSort,&_DATA,0,&topthreadID);
	WaitForSingleObject(topthread,INFINITE);
	CloseHandle(topthread);
	UnmapViewOfFile(filemap);
	CloseHandle(filemapping);
	CloseHandle(f);
	ofstream fdisplay("data_sorted.txt",ios_base::out);
	ifstream fdat("data_proc.dat",ios_base::binary);
	int k;
	for(int i=0;i<datanum;i++){
		fdat.read(reinterpret_cast<char *>(&k),sizeof(k));
		fdisplay<<k<<'\n';
	}
	fdat.close();
	fdisplay.close();
	return 0;
}
 