#include<iostream>
#include<fstream>
#include<windows.h>
#include<queue>
#include<ctime>
using namespace std;
#define BkClerkNum 3					//ÒøÐÐ¹ñÔ±ÈËÊý
#define CustmNum 8						//¹Ë¿ÍÈËÊý
int custmnumresult=0;					//¹Ë¿ÍÄÃµ½µÄºÅ
typedef struct{							//²âÊÔÊý¾Ý¸ñÊ½(¹Ë¿ÍÐÅÏ¢)
	int Num;							//¹Ë¿ÍÐòºÅ£¨²âÊÔÎÄ¼þÖÐµÄÐòºÅ£©
	int EnterTime;						//½øÈëÒøÐÐÊ±¼ä
	int RequireTime;					//ÐèÒª·þÎñÊ±¼ä
}CustmInfo;
CustmInfo custminfo[CustmNum];
typedef struct{							//¹Ë¿ÍÐÅÏ¢£ºÄÃµ½µÄºÅ£¬½øÈëÒøÐÐÊ±¼ä£¬·þÎñ¹ñÔ±ÐòºÅ£¬¿ªÊ¼·þÎñÊ±¼ä£¬Àë¿ªÒøÐÐÊ±¼ä
	int Num;
	int EnterTime;
	int BeginServedTime;
	int ServerNum;
	int LeaveTime;
}CustmResult;
CustmResult custmresult[CustmNum];

//¶ÁÈ¡¹Ë¿ÍÐÅÏ¢
void ReadCustmInfo(){
	ifstream f("custminfo.txt",ios_base::in);
	int b[3];
	for(int k=0;k<CustmNum;k++){
		for(int i=0;i<3;i++)
			f>>b[i];
		custminfo[k].Num=b[0];
		custminfo[k].EnterTime=b[1];
		custminfo[k].RequireTime=b[2];
	}
	f.close();
}
int waiting=0;				//µÈ´ýµÄ¹Ë¿ÍÈËÊý
double Action=0;			//Ê±¼äÆðµã
int CustmDone=0;				//Íê³É·þÎñµÄ¹Ë¿ÍÊý
queue<CustmInfo>CustmQueue;	//½øÈëÒøÐÐµÄ¹Ë¿Í¶ÓÁÐ
HANDLE custm_sema=CreateSemaphore(NULL,0,CustmNum,TEXT("number_of_customers"));		//ÓÃÓÚ¹Ë¿ÍµÈ´ý¶ÓÁÐµÄÐÅºÅÁ¿£ºÃ»ÓÐ¹Ë¿ÍÔÚµÈ´ýÔò×èÈû¹ñÔ±
HANDLE custm_mutex=CreateMutex(NULL,FALSE,TEXT("customer_mutex"));					//ÓÃÓÚ¹Ë¿ÍÏß³ÌµÄ»¥³âÁ¿£º¹Ë¿ÍÄÃºÅ
HANDLE clerk_mutex=CreateMutex(NULL,FALSE,TEXT("clerk_mutex"));					    //ÓÃÓÚ¹ñÔ±Ïß³ÌµÄ»¥³âÁ¿£º¹ñÔ±½ÐºÅ

//¹ñÔ±½ø³Ì
DWORD WINAPI BankClerk(LPVOID lpParam){
	int *ClerkNum=(int *)lpParam;		//¹ñÔ±ÐòºÅ
	while(CustmDone<CustmNum){ //¹ñÔ±Ò»Ö±ÔÚ¹¤×÷£¬ÒªÃ´Îª¹Ë¿Í·þÎñ£¬ÒªÃ´µÈ´ý½ÐºÅ,Ö±ÖÁ¹Ë¿ÍÒÑ¾­È«²¿Íê³É·þÎñ
		WaitForSingleObject(custm_sema,INFINITE);
		WaitForSingleObject(clerk_mutex,INFINITE);
		CustmInfo customer=CustmQueue.front();	//µ±Ç°±»·þÎñµÄ¹Ë¿Í
		CustmQueue.pop();
		CustmDone+=1;
		waiting-=1;
		int t0=(int(clock())-Action)/1000;
		cout<<"Time:"<<t0<<"  Customer "<<custmresult[customer.Num-1].Num<<" gets served by Clerk "<<*ClerkNum<<'.'<<endl;
		custmresult[customer.Num-1].ServerNum=*ClerkNum;
		custmresult[customer.Num-1].BeginServedTime=t0;
		ReleaseMutex(clerk_mutex);
		Sleep(customer.RequireTime*1000);
		t0=(int(clock())-Action)/1000;
		cout<<"Time:"<<t0<<"  Customer "<<custmresult[customer.Num-1].Num<<" leaves the bank."<<endl;
		custmresult[customer.Num-1].LeaveTime=t0;
	}
	return 0;
}

//¹Ë¿Í½ø³Ì
DWORD WINAPI Customer(LPVOID lpParam){
	CustmInfo *customer=(CustmInfo *)lpParam;
	Sleep(customer->EnterTime*1000);
	WaitForSingleObject(custm_mutex,INFINITE);
	custmresult[customer->Num-1].Num=++custmnumresult;
	CustmQueue.push(*customer);
	waiting+=1;
	int t0=(int(clock())-Action)/1000;
	cout<<"Time:"<<t0<<"  Customer "<<custmresult[customer->Num-1].Num<<" enters the bank."<<endl;
	custmresult[customer->Num-1].EnterTime=t0;
	ReleaseMutex(custm_mutex);
	ReleaseSemaphore(custm_sema,1,NULL);
	return 0;
}

int main(){
	ReadCustmInfo();
	HANDLE hThread[CustmNum+BkClerkNum];	//Ïß³Ì¾ä±ú
	DWORD dwCustmId[CustmNum];				//¹Ë¿ÍÏß³Ì±êÊ¶
	DWORD dwClerkId[BkClerkNum];			//¹ñÔ±Ïß³Ì±êÊ¶ 
	int ClerkNum[BkClerkNum];//				//¹ñÔ±ÐòºÅ
	for(int k=0;k<BkClerkNum;k++)
		ClerkNum[k]=k+1;
	Action=int(clock());					//¿ªÊ¼¼ÆÊ±
	for(int k=0;k<CustmNum;k++){
		hThread[k]=CreateThread(NULL,0,Customer,custminfo+k,0,&dwCustmId[k]);
		if(hThread[k]==NULL){
			cout<<"Create thread error!\n";
			return -1;
		}
	}
	for(int k=0;k<BkClerkNum;k++){
		hThread[k+CustmNum]=CreateThread(NULL,0,BankClerk,ClerkNum+k,0,&dwClerkId[k]);
		if(hThread[k+CustmNum]==NULL){
			cout<<"Create thread error!\n";
			return -1;
		}
	}
	WaitForMultipleObjects(CustmNum+BkClerkNum,hThread,true,INFINITE);
	for(int k=0;k<CustmNum+BkClerkNum;k++)
		CloseHandle(hThread[k]);
	cout<<endl<<endl;
	cout<<"CustomerNumber\tEnterTime\tBeginServedTime\tLeaveTime\tClerkNum\n";
	for(int k=0;k<CustmNum;k++)
		cout<<custmresult[k].Num<<"\t\t"<<custmresult[k].EnterTime<<"\t\t"<<custmresult[k].BeginServedTime<<"\t\t"<<custmresult[k].LeaveTime<<"\t\t"<<custmresult[k].ServerNum<<endl;
	return 0;
}

