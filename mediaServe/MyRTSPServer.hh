
#ifndef _MY_RTSP_SERVER_HH
#define _MY_RTSP_SERVER_HH

#include "RTSPServerSupportingHTTPStreaming.hh"


class MyRTSPServer: public RTSPServerSupportingHTTPStreaming {
public:
  static MyRTSPServer* createNew(UsageEnvironment& env, Port ourPort,
				      UserAuthenticationDatabase* authDatabase,
				      unsigned reclamationTestSeconds = 65);

protected:
  MyRTSPServer(UsageEnvironment& env, int ourSocket, Port ourPort,
		    UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds);
  virtual ~MyRTSPServer();

protected:
  virtual ServerMediaSession*
  lookupServerMediaSession(char const* streamName, Boolean isFirstLookupInSession);
};

#endif
