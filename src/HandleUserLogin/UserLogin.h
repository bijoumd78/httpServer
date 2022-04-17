#pragma once

#include <Poco/Data/Session.h>
#include <string>
#include <string_view>

namespace HandleUserLogin {
    struct User_
    {
        std::string email;
        std::string password;
    };

    class UserLoginDB
    {
    public:
        UserLoginDB();

        void insertUser(const User_& user)const;
        [[nodiscard]] bool searchUser(const User_& user)const;
        [[nodiscard]] bool searchUserEmail(const User_& user)const;

    private:
        // Utility function
        std::string gethashKey(std::string_view message)const;

        std::string db_{ "SQLite" };
        std::string dbName_{ "loginData.db" };
        mutable Poco::Data::Session session_;
        // HMAC needs a passphrase
        std::string passphrase_{ "s3cr3t" };
    };
}
