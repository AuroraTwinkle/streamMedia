#include <BasicUsageEnvironment.hh>
#include "MyRTSPServer.hh"


 int main() {
	 TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	 UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

	 UserAuthenticationDatabase* authDB = NULL;

	 RTSPServer* rtspServer;
	 portNumBits rtspServerPortNum = 554;
	 rtspServer = MyRTSPServer::createNew(*env, rtspServerPortNum, authDB);
	 if (rtspServer == NULL) {//rtsp服务器注册失败
		 rtspServerPortNum = 8554;
		 rtspServer = MyRTSPServer::createNew(*env, rtspServerPortNum, authDB);
	 }
	 if (rtspServer == NULL) {//rtsp服务器注册失败
		 *env << "rtsp服务器注册失败: " << env->getResultMsg() << "\n";
		 exit(1);
	 }

	 *env << "欢迎使用MyRTSPServer(design by live555)\n";
	 char* urlPrefix = rtspServer->rtspURLPrefix();
	 *env << "使用下面的url来播放流\n\t"
		 << urlPrefix << "<filename>\n";
	 *env << "(目前仅支持mkv格式的视频)\n";

	 env->taskScheduler().doEventLoop();

	 return 0;
}
