#pragma once

#include <gal_common.h>
#include <Common.h>

#include "network/NetworkService.h"
#include "network/Client.h"
#include "network/ConnectionState.h"
#include "network/Transporter.h"
#include "network/NetworkEventListener.h"
#include "foundation/TCPSocket.h"
#include "foundation/LoggingService.h"
#include "foundation/Regulator.h"
#include "foundation/ResourceManager.h"

#include "MRSGmProto/ClientMRSGmProtoProcedureBinaryParser.h"
#include "MRSGmProto/ClientMRSGmProtoProcedureBinaryGenerator.h"
#include "MRSGmProto/ClientMRSGmProtoSender.h"
#include "MRSGmProto/ClientMRSGmProtoContract.h"
#include "MRSGmProto/IClientMRSGmProtoReceiver.h"

class MRSClient;

//<sender, receiver, parser>
typedef MRSGmProto::ClientMRSGmProtoContract<MRSClient, MRSGmProto::ClientMRSGmProtoSender<MRSGmProto::ClientMRSGmProtoProcedureBinaryGenerator>, MRSGmProto::ClientMRSGmProtoProcedureBinaryParser>	CMRSGmContract;

namespace GmClProto {
	class destination_t;
};

namespace proto = MRSGmProto;
class MRSClient
  : public MRSGmProto::IClientMRSGmProtoReceiver
{
public:
  MRSClient( oneup::Contract* pContract);
  virtual ~MRSClient();

  int initialize( CMRSGmContract& rContract, oneup::Client& rConnect);
  int isConnect() const;
  bool onOpen();
  bool onClose();
  int proceed();
  int terminate();

  uint64_t gen_id() const {
		static uint64_t value = 0ull;
#if defined( __GNUC__ ) && ( __GNUC__ == 4 && __GNUC_MINOR__ >= 2 )
		return __sync_fetch_and_add ( &value, 1 );
#else
		static boost::mutex guard;
		boost::mutex::scoped_lock lock( guard );
		return value++;
#endif 
  }

  static GmClProto::destination_t destinationToGmCl(const MRSGmProto::destination_t& in);
	static MRSGmProto::destination_t getAnnounceGroup();

  //プロトコル実装
  bool sendC_MRSGM_GMSVINFO_REQ();
  void C_MRSGM_GMSVINFO_REQ_Response( oneup::s32 iResult, oneup::string strError);
  void addGroup_Response( MRSGmProto::request_id_t, MRSGmProto::result_t result);
  void delGroup_Response( MRSGmProto::request_id_t, MRSGmProto::result_t result);
  void addMember_Response( MRSGmProto::request_id_t, MRSGmProto::result_t result);
  void delMember_Response( MRSGmProto::request_id_t, MRSGmProto::result_t result);
  void post_Response( MRSGmProto::request_id_t, MRSGmProto::result_t result);
  void getMembers_Response( MRSGmProto::request_id_t, MRSGmProto::result_t result, MRSGmProto::destinations);

  void C_MRSGM_CHAR_LOGIN_REQ_Response( MRSGmProto::cuid_t, MRSGmProto::destinations);
  void S_MRSGM_NOTIFY_GROUPS( MRSGmProto::cuid_t, MRSGmProto::destinations);
  void S_MRSGM_NOTIFY_MESSAGE( MRSGmProto::receivers_t, MRSGmProto::destination_t, MRSGmProto::blob_t);
  void S_MRSGM_NOTIFY_YOU_ARE_INVITED( MRSGmProto::receivers_t, MRSGmProto::character_info_list_t, MRSGmProto::destinations); 
  void S_MRSGM_NOTIFY_YOU_ARE_KICKED( MRSGmProto::receivers_t, MRSGmProto::character_info_list_t, MRSGmProto::destinations);

private:
  const CONNECTUID m_connectUID; 
  oneup::Client* m_pClient;
  CMRSGmContract* m_pSender;

  bool m_bConnect;
  bool m_bRecvGmsvInfoRep;
	time_t	m_tiInitializeRequestTime;
};
