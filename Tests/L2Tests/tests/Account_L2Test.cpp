#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>
#include <limits>
#include <mutex>
#include <thread>

#include "L2Tests.h"
#include "L2TestsMock.h"
#include "interfaces/IAccount.h"

//#define TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);

using namespace WPEFramework;

class Account_L2Test : public L2TestMocks {
protected:
    Exchange::IAccount* m_AccountPlugin = nullptr;
    PluginHost::IShell* m_controller_Account = nullptr;
    Core::ProxyType<RPC::InvokeServerType<1, 0, 4>> Account_Engine;
    Core::ProxyType<RPC::CommunicatorClient> Account_Client;

    Account_L2Test()
        : L2TestMocks()
    {
        uint32_t status = Core::ERROR_GENERAL;

        status = ActivateService("org.rdk.PersistentStore");
        EXPECT_EQ(Core::ERROR_NONE, status);

        status = ActivateService("org.rdk.Account");
        EXPECT_EQ(Core::ERROR_NONE, status);

        status = CreateAccountInterfaceObject();
        EXPECT_EQ(Core::ERROR_NONE, status);
    }

    ~Account_L2Test() override
    {
        if (m_AccountPlugin != nullptr)
        {
            m_AccountPlugin->Release();
            m_AccountPlugin = nullptr;
        }

        if (m_controller_Account != nullptr)
        {
            m_controller_Account->Release();
            m_controller_Account = nullptr;
        }

        DeactivateService("org.rdk.Account");
        DeactivateService("org.rdk.PersistentStore");
    }

    uint32_t CreateAccountInterfaceObject()
    {
        uint32_t return_value = Core::ERROR_GENERAL;

        Account_Engine = Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create();
        Account_Client = Core::ProxyType<RPC::CommunicatorClient>::Create(
            Core::NodeId("/tmp/communicator"),
            Core::ProxyType<Core::IIPCServer>(Account_Engine));

#if ((THUNDER_VERSION == 2) || ((THUNDER_VERSION == 4) && (THUNDER_VERSION_MINOR == 2)))
        Account_Engine->Announcements(Account_Client->Announcement());
#endif

        if (!Account_Client.IsValid()) {
            TEST_LOG("Invalid Account_Client");
        } else {
            m_controller_Account = Account_Client->Open<PluginHost::IShell>("org.rdk.Account", ~0, 3000);

            if (m_controller_Account != nullptr) {
                m_AccountPlugin = m_controller_Account->QueryInterface<Exchange::IAccount>();
                if (m_AccountPlugin != nullptr) {
                    return_value = Core::ERROR_NONE;
                    TEST_LOG("Successfully created Account Plugin Interface");
                } else {
                    TEST_LOG("Failed to get Account Plugin Interface");
                }
            } else {
                TEST_LOG("Failed to get Account controller");
            }
        }

        return return_value;
    }
};

TEST_F(Account_L2Test, GetDefaultLastCheckoutResetTime_Success)
{
    if (m_AccountPlugin)
    {
        const uint64_t expectedDefaultResetTime = 0;
        Exchange::IAccount::GetLastCheckoutResetTimeResult result{};
        result.resetTime = 1;

        EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->GetLastCheckoutResetTime(result));
        EXPECT_EQ(expectedDefaultResetTime, result.resetTime);
    }
}

TEST_F(Account_L2Test, SetAndGetLastCheckoutResetTime_Success)
{
    if (m_AccountPlugin)
    {
        const uint64_t expectedResetTime = 123456789ULL;
        Exchange::IAccount::GetLastCheckoutResetTimeResult result{};
        result.resetTime = 0;

        EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->SetLastCheckoutResetTime(expectedResetTime));
        EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->GetLastCheckoutResetTime(result));
        EXPECT_EQ(expectedResetTime, result.resetTime);
    }
}

TEST_F(Account_L2Test, SetAndGetLastCheckoutResetTime_ZeroBoundary)
{
    if (m_AccountPlugin)
    {
        const uint64_t expectedResetTime = 0;
        Exchange::IAccount::GetLastCheckoutResetTimeResult result{};
        result.resetTime = 1;

        EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->SetLastCheckoutResetTime(expectedResetTime));
        EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->GetLastCheckoutResetTime(result));
        EXPECT_EQ(expectedResetTime, result.resetTime);
    }

}

TEST_F(Account_L2Test, SetAndGetLastCheckoutResetTime_MaxUint64Boundary)
{
    if (m_AccountPlugin)
    {
        const uint64_t expectedResetTime = std::numeric_limits<uint64_t>::max();
        Exchange::IAccount::GetLastCheckoutResetTimeResult result{};
        result.resetTime = 0;

        EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->SetLastCheckoutResetTime(expectedResetTime));
        EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->GetLastCheckoutResetTime(result));
        EXPECT_EQ(expectedResetTime, result.resetTime);
    }
}

TEST_F(Account_L2Test, SetLastCheckoutResetTime_OverwriteValue)
{
    if (m_AccountPlugin)
    {
        const uint64_t firstResetTime = 111ULL;
        const uint64_t secondResetTime = 222ULL;
        Exchange::IAccount::GetLastCheckoutResetTimeResult result{};
        result.resetTime = 0;

        EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->SetLastCheckoutResetTime(firstResetTime));
        EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->SetLastCheckoutResetTime(secondResetTime));
        EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->GetLastCheckoutResetTime(result));
        EXPECT_EQ(secondResetTime, result.resetTime);
    }
}