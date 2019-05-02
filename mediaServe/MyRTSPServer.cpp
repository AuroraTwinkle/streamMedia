#include "MyRTSPServer.hh"
#include <liveMedia.hh>
#include <string.h>

MyRTSPServer*
MyRTSPServer::createNew(UsageEnvironment& env, Port ourPort,
			     UserAuthenticationDatabase* authDatabase,
			     unsigned reclamationTestSeconds) {
  int ourSocket = setUpOurSocket(env, ourPort);
  if (ourSocket == -1) return NULL;

  return new MyRTSPServer(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds);
}

MyRTSPServer::MyRTSPServer(UsageEnvironment& env, int ourSocket,
				     Port ourPort,
				     UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds)
  : RTSPServerSupportingHTTPStreaming(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds) {
}

MyRTSPServer::~MyRTSPServer() {
}

static ServerMediaSession* createNewSMS(UsageEnvironment& env,
					char const* fileName, FILE* fid);

ServerMediaSession* MyRTSPServer
::lookupServerMediaSession(char const* streamName, Boolean isFirstLookupInSession) {
  // ��鱾���Ƿ���ڸ�����ԭ�ļ�
  FILE* fid = fopen(streamName, "rb");
  Boolean fileExists = fid != NULL;

  //����Ƿ���ڸ�����Ӧ��sms�Ự
  ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(streamName);
  Boolean smsExists = sms != NULL;

  if (!fileExists) {
    if (smsExists) {
      removeServerMediaSession(sms);
      sms = NULL;
    }

    return NULL;
  } else {
    if (smsExists && isFirstLookupInSession) { 
      removeServerMediaSession(sms); 
      sms = NULL;
    } 

    if (sms == NULL) {
      sms = createNewSMS(envir(), streamName, fid); 
      addServerMediaSession(sms);
    }

    fclose(fid);
    return sms;
  }
}


struct MatroskaDemuxCreationState {
  MatroskaFileServerDemux* demux;
  char watchVariable;
};
static void onMatroskaDemuxCreation(MatroskaFileServerDemux* newDemux, void* clientData) {
  MatroskaDemuxCreationState* creationState = (MatroskaDemuxCreationState*)clientData;
  creationState->demux = newDemux;
  creationState->watchVariable = 1;
}




#define NEW_SMS(description) do {\
char const* descStr = description\
    ", streamed by MyRTSPServer";\
sms = ServerMediaSession::createNew(env, fileName, fileName, descStr);\
} while(0)

static ServerMediaSession* createNewSMS(UsageEnvironment& env,
					char const* fileName, FILE* /*fid*/) {
  //��ȡ�ļ���׺��
  char const* extension = strrchr(fileName, '.');
  if (extension == NULL) return NULL;

  ServerMediaSession* sms = NULL;
  Boolean const reuseSource = False;
  if (strcmp(extension, ".mkv") == 0) {
    
    OutPacketBuffer::maxSize = 300000; 
    NEW_SMS("Matroska video+audio+(optional)subtitles");

    MatroskaDemuxCreationState creationState;
    creationState.watchVariable = 0;
    MatroskaFileServerDemux::createNew(env, fileName, onMatroskaDemuxCreation, &creationState);
    env.taskScheduler().doEventLoop(&creationState.watchVariable);

    ServerMediaSubsession* smss;
    while ((smss = creationState.demux->newServerMediaSubsession()) != NULL) {
      sms->addSubsession(smss);
    }
  }

  return sms;
}
