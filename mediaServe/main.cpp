#include <BasicUsageEnvironment.hh>
#include "MyRTSPServer.hh"


 int main() {
	 TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	 UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

	 UserAuthenticationDatabase* authDB = NULL;

	 RTSPServer* rtspServer;
	 portNumBits rtspServerPortNum = 554;
	 rtspServer = MyRTSPServer::createNew(*env, rtspServerPortNum, authDB);
	 if (rtspServer == NULL) {//rtsp������ע��ʧ��
		 rtspServerPortNum = 8554;
		 rtspServer = MyRTSPServer::createNew(*env, rtspServerPortNum, authDB);
	 }
	 if (rtspServer == NULL) {//rtsp������ע��ʧ��
		 *env << "rtsp������ע��ʧ��: " << env->getResultMsg() << "\n";
		 exit(1);
	 }

	 *env << "��ӭʹ��MyRTSPServer(design by live555)\n";
	 char* urlPrefix = rtspServer->rtspURLPrefix();
	 *env << "ʹ�������url��������\n\t"
		 << urlPrefix << "<filename>\n";
	 *env << "(Ŀǰ��֧��mkv��ʽ����Ƶ)\n";

	 env->taskScheduler().doEventLoop();

	 return 0;
}
