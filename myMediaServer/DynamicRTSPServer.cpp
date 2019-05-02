#include "DynamicRTSPServer.hh"
#include <liveMedia.hh>
#include <string.h>

DynamicRTSPServer*
DynamicRTSPServer::createNew(UsageEnvironment& env, Port ourPort,
			     UserAuthenticationDatabase* authDatabase,
			     unsigned reclamationTestSeconds) {
  int ourSocket = setUpOurSocket(env, ourPort);
  if (ourSocket == -1) return NULL;

  return new DynamicRTSPServer(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds);
}

DynamicRTSPServer::DynamicRTSPServer(UsageEnvironment& env, int ourSocket,
				     Port ourPort,
				     UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds)
  : RTSPServerSupportingHTTPStreaming(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds) {
}

DynamicRTSPServer::~DynamicRTSPServer() {
}

static ServerMediaSession* createNewSMS(UsageEnvironment& env,
					char const* fileName, FILE* fid); // forward

ServerMediaSession* DynamicRTSPServer
::lookupServerMediaSession(char const* streamName, Boolean isFirstLookupInSession) {
  // First, check whether the specified "streamName" exists as a local file:
  FILE* fid = fopen(streamName, "rb");
  Boolean fileExists = fid != NULL;

  // Next, check whether we already have a "ServerMediaSession" for this file:
  ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(streamName);
  Boolean smsExists = sms != NULL;

  // Handle the four possibilities for "fileExists" and "smsExists":
  if (!fileExists) {
    if (smsExists) {
      // "sms" was created for a file that no longer exists. Remove it:
      removeServerMediaSession(sms);
      sms = NULL;
    }

    return NULL;
  } else {
    if (smsExists && isFirstLookupInSession) { 
      // Remove the existing "ServerMediaSession" and create a new one, in case the underlying
      // file has changed in some way:
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

// Special code for handling Matroska files:
struct MatroskaDemuxCreationState {
  MatroskaFileServerDemux* demux;
  char watchVariable;
};
static void onMatroskaDemuxCreation(MatroskaFileServerDemux* newDemux, void* clientData) {
  MatroskaDemuxCreationState* creationState = (MatroskaDemuxCreationState*)clientData;
  creationState->demux = newDemux;
  creationState->watchVariable = 1;
}
// END Special code for handling Matroska files:



#define NEW_SMS(description) do {\
char const* descStr = description\
    ", streamed by the LIVE555 Media Server";\
sms = ServerMediaSession::createNew(env, fileName, fileName, descStr);\
} while(0)

static ServerMediaSession* createNewSMS(UsageEnvironment& env,
					char const* fileName, FILE* /*fid*/) {
  // Use the file name extension to determine the type of "ServerMediaSession":
  char const* extension = strrchr(fileName, '.');
  if (extension == NULL) return NULL;

  ServerMediaSession* sms = NULL;
  Boolean const reuseSource = False;
  if (strcmp(extension, ".mkv") == 0 || strcmp(extension, ".webm") == 0) {
    // Assumed to be a Matroska file (note that WebM ('.webm') files are also Matroska files)
    OutPacketBuffer::maxSize = 300000; // allow for some possibly large VP8 or VP9 frames
    NEW_SMS("Matroska video+audio+(optional)subtitles");

    // Create a Matroska file server demultiplexor for the specified file.
    // (We enter the event loop to wait for this to complete.)
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
