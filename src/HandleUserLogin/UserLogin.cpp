#include "UserLogin.h"
#include <Poco/Data/Session.h>
#include <Poco/HMACEngine.h>
#include <Poco/SHA1Engine.h>
#include <Logger.h>

namespace HandleUserLogin
{
	using namespace Poco::Data::Keywords;
	using Poco::Data::Session;
	using Poco::DigestEngine;
	using Poco::HMACEngine;
	using Poco::SHA1Engine;
	using Poco::Data::Statement;


	UserLoginDB::UserLoginDB() :
		session_(db_, dbName_)
	{
		session_ << "CREATE TABLE IF NOT EXISTS User (Email VARCHAR, Password VARCHAR(40))", now;
	}

	void UserLoginDB::insertUser(const User_& user)const
	{
		Common::Logging::Logger::log("error", "UserLogin", -1, "Registring new user. ");

		Statement insert(session_);
		const auto& pass = gethashKey(user.password);
		insert << "INSERT INTO User VALUES(?, ?)", useRef(user.email), useRef(pass);
		insert.execute();
	}

	bool UserLoginDB::searchUser(const User_& user)const
	{
		bool isFound{ false };
		User_ l_user;
		Statement select(session_);
		std::stringstream ss;
		ss << "SELECT Email, Password FROM User WHERE Email like '%" << user.email << "%'";
		select << ss.str(), into(l_user.email), into(l_user.password), now;
		select.execute();

		if (!l_user.email.empty() && (Poco::icompare(l_user.password, gethashKey(user.password)) == 0))
		{
			Common::Logging::Logger::log("information", "UserLogin", -1, "User authenticated!");
			isFound = true;
		}
		else
		{
			Common::Logging::Logger::log("error", "UserLogin", -1, "User not authenticated! ");
		}

		return isFound;
	}

	std::string UserLoginDB::gethashKey(std::string_view message) const
	{
		// we'll compute a HMAC-SHA1
		HMACEngine<SHA1Engine> hmac(passphrase_); 
		hmac.update(message.data());

		// finish HMAC computation and obtain digest
		const DigestEngine::Digest& digest = hmac.digest();

		// convert to a string of hexadecimal numbers
		std::string digestString{ DigestEngine::digestToHex(digest) };

		return digestString;
	}

}