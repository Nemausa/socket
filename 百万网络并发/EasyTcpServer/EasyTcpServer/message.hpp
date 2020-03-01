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
* @author Morris
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
	CMD_ERROR
};

struct DataHeader
{
	short length_;
	short cmd_;
};

struct Login : public DataHeader
{
	Login() { cmd_ = CMD_LOGIN; length_ = sizeof(Login); }
	char username_[32];
	char passwd_[32];
	char data[32];
};

struct LoginResult :public DataHeader
{
	LoginResult() { cmd_ = CMD_LOGIN_RESULT, length_ = sizeof(LoginResult); result_ = 0; }
	int result_;
	char data[92];
};

struct SignOut :public DataHeader
{
	SignOut() { cmd_ = CMD_SIGNOUT, length_ = sizeof(SignOut); }
	char username_[32];
};

struct SignOutResult :public DataHeader
{
	SignOutResult() { cmd_ = CMD_SIGNOUT_RESULT, length_ = sizeof(SignOutResult); result_ = 0; }
	int result_;
};

struct NewUserJoin :public DataHeader
{
	NewUserJoin() { cmd_ = CMD_NEW_USER_JOIN, length_ = sizeof(NewUserJoin); id_socket = 0; }
	int id_socket;
};

#endif
