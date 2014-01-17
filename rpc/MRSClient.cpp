#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/fusion/include/comparison.hpp>
#include <boost/spirit/include/karma.hpp>

#include "gal_common.h"
#include "Common.h"
#include "CSvUtil.h"

#include "network/NetworkService.h"
#include "network/Client.h"
#include "network/ConnectionState.h"
#include "network/Serializer.h"
#include "network/Transporter.h"
#include "network/NetworkEventListener.h"
#include "foundation/TCPSocket.h"
#include "foundation/LoggingService.h"
#include "foundation/Regulator.h"
#include "foundation/ResourceManager.h"

#include "message_server/container/destination.hpp"

#include "MRSClient.h"

MRSClient::MRSClient(oneup::Contract* pContract)
  : IClientMRSGmProtoReceiver(pContract)
  , m_connectUID(CSvUtil::createUID64())
{
  m_pClient			= NULL;
	m_pSender			= NULL;
	m_bConnect			= false;
	m_bRecvGmsvInfoRep	= false;
	m_tiInitializeRequestTime = 0;
}

MRSClient::~MRSClient()
{
}

proto::destination_t MRSClient::getAnnounceGroup()
{
	proto::destination_t	group;
	group.category = E_CHAT_CATEGORY_ANNOUNCE;
	group.channel.upper = 0;
	group.channel.lower = 0;
	group.topic.upper = 0;
	group.topic.lower = 0;
	group.auxiliary.upper = 0;
	group.auxiliary.lower = 0;
	return group;
}

int MRSClient::initialize(CMRSGmContract& rContract, oneup::Client& rConnect)
{
	m_pClient = &rConnect;
	m_pSender = &rContract;
	try {
		rConnect.addContract("MRSGMPROTO", m_pSender);
		rConnect.getTransporter().setSocket(new oneup::foundation::TCPSocket());
	}
	catch (std::bad_alloc& r) {
		eprint("___ %s\n", r.what());
		return -1024;
	}
	return 0;
}

bool MRSClient::isConnect() const
{
	return (m_bConnect & m_bRecvGmsvInfoRep);
}

bool MRSClient::onOpen()
{
	sendC_MRSGM_GMSVINFO_REQ();
	return true;
}

int MRSClient::terminate()
{
	m_pClient->terminate();
	return 0;
}

int MRSClient::proceed()
{
	if (false == isConnect()) {
		void*	pState = m_pClient->getFSM().getCurrentState();
		if (pState == oneup::CSConnecting::getInstance()) {
		} else if (pState == oneup::CSEstablished::getInstance()) {
			// Establishedなら、接続が完了している
			m_bConnect = true;
		} else if (pState == oneup::CSException::getInstance()) {
			// Exceptionの場合、タイムアウトなどのエラーが発生している
			return -100;
		} else if (pState == oneup::CSTerminate::getInstance()) {
			return -200;
		} else if (pState == oneup::CSClosing::getInstance()) {
			return -300;
		}
	} else {
	}
	return 0;
}

bool MRSClient::sendC_MRSGM_GMSVINFO_REQ()
{
  if (0 != m_tiInitializeRequestTime)     return false;   // ２重リクエストの防止
  m_tiInitializeRequestTime = time(NULL);

  getSender().C_MRSGM_GMSVINFO_REQ(0, 247, "127.0.0.1", 8000);
  return true;
}

void MRSClient::C_MRSGM_CHAR_LOGIN_REQ_Response( 
  proto::cuid_t cuid,
	proto::destinations groups
) {
}

void MRSClient::S_MRSGM_NOTIFY_MESSAGE(
  proto::receivers_t receivers_oue,
  proto::destination_t groups_oue,
  proto::blob_t notify_blob
) {
}

void MRSClient::C_MRSGM_GMSVINFO_REQ_Response(oneup::s32 iResult, oneup::string strError)
{
}

void MRSClient::S_MRSGM_NOTIFY_GROUPS(
	MRSGmProto::cuid_t cuid,
	MRSGmProto::destinations groups
) {
}

void MRSClient::S_MRSGM_NOTIFY_YOU_ARE_INVITED(
	MRSGmProto::receivers_t receivers_oue,
	MRSGmProto::destinations groups_oue
) {
}

void MRSClient::S_MRSGM_NOTIFY_YOU_ARE_KICKED(
	MRSGmProto::receivers_t receivers_oue,
	MRSGmProto::destinations groups_oue
) {
}

void MRSClient::S_MRSGM_NOTIFY_THEY_ARE_INVITED(
	MRSGmProto::receivers_t receivers_oue,
	MRSGmProto::character_info_list_t infos,
	MRSGmProto::destinations groups
) {
}

void MRSClient::S_MRSGM_NOTIFY_THEY_ARE_KICKED(
	MRSGmProto::receivers_t receivers_oue,
	MRSGmProto::character_info_list_t infos,
	MRSGmProto::destinations groups
) {
}

void MRSClient::addGroup_Response(
	MRSGmProto::request_id_t id,
	MRSGmProto::result_t result
) {
}
void MRSClient::addMember_Response(
  MRSGmProto::request_id_t id,
  MRSGmProto::result_t result
) {
}

void MRSClient::delMember_Response(
  MRSGmProto::request_id_t id,
  MRSGmProto::result_t result
) {
}

void MRSClient::delGroup_Response(
  MRSGmProto::request_id_t id,
  MRSGmProto::result_t result
) {
}

void MRSClient::post_Response(
  MRSGmProto::request_id_t id,
  MRSGmProto::result_list result
) {
}

void modifyGroup_Response(
  MRSGmProto::request_id_t, 
  MRSGmProto::result_t result
) {
}
