#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

/**
* @file meaasge.hpp
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2020-02-21
* @author morris
* contact: tappanmorris@outlook.com
*
*/

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_SIGNOUT,
	CMD_SIGNOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_HEART_C2S,
	CMD_HEART_S2C,
	CMD_ERROR
};

struct NetDataHeader
{
	short length_;
	short cmd_;
};

struct NetLogin : public NetDataHeader
{
	NetLogin() { cmd_ = CMD_LOGIN; length_ = sizeof(NetLogin); }
	char username_[32];
	char passwd_[32];
	char data[32];
};

struct NetLoginR :public NetDataHeader
{
	NetLoginR() { cmd_ = CMD_LOGIN_RESULT, length_ = sizeof(NetLoginR); result_ = 0; }
	int result_;
	char data[92];
};

struct NetMsgSignOut :public NetDataHeader
{
	NetMsgSignOut() { cmd_ = CMD_SIGNOUT, length_ = sizeof(NetMsgSignOut); }
	char username_[32];
};

struct NetSignOutR :public NetDataHeader
{
	NetSignOutR() { cmd_ = CMD_SIGNOUT_RESULT, length_ = sizeof(NetSignOutR); result_ = 0; }
	int result_;
};

struct NetNewUserJoin :public NetDataHeader
{
	NetNewUserJoin() { cmd_ = CMD_NEW_USER_JOIN, length_ = sizeof(NetNewUserJoin); id_socket = 0; }
	int id_socket;
};

struct Net_C2S_Heart:public NetDataHeader
{
	Net_C2S_Heart()
	{
		length_ = sizeof(Net_C2S_Heart);
		cmd_ = CMD_HEART_C2S;
	}
};

struct Net_S2C_Heart :public NetDataHeader
{
	Net_S2C_Heart()
	{
		length_ = sizeof(Net_S2C_Heart);
		cmd_ = CMD_HEART_S2C;
	}
};


#endif
