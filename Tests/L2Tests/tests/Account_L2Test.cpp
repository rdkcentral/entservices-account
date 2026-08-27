/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2026 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <chrono>
#include <condition_variable>
#include <limits>
#include <mutex>
#include <thread>

#include "L2Tests.h"
#include "L2TestsMock.h"
#include "interfaces/IAccount.h"

using namespace Thunder;

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

        if (remove("/tmp/secure/persistent/rdkservicestore") != 0 && errno != ENOENT)
            TEST_LOG("Failed to remove existing persistent store file, error: %d: %s", errno, strerror(errno));

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
    EXPECT_TRUE(m_AccountPlugin != nullptr);

    if (!m_AccountPlugin)
        return;

    const uint64_t expectedDefaultResetTime = 0;
    Exchange::IAccount::GetLastCheckoutResetTimeResult result{};
    result.resetTime = 1;

    EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->GetLastCheckoutResetTime(result));
    EXPECT_EQ(expectedDefaultResetTime, result.resetTime);
}

TEST_F(Account_L2Test, SetAndGetLastCheckoutResetTime_Success)
{
    EXPECT_TRUE(m_AccountPlugin != nullptr);

    if (!m_AccountPlugin)
        return;

    const uint64_t expectedResetTime = 123456789ULL;
    Exchange::IAccount::GetLastCheckoutResetTimeResult result{};
    result.resetTime = 0;

    EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->SetLastCheckoutResetTime(expectedResetTime));
    EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->GetLastCheckoutResetTime(result));
    EXPECT_EQ(expectedResetTime, result.resetTime);
}

TEST_F(Account_L2Test, SetAndGetLastCheckoutResetTime_ZeroBoundary)
{
    EXPECT_TRUE(m_AccountPlugin != nullptr);

    if (!m_AccountPlugin)
        return;

    const uint64_t expectedResetTime = 0;
    Exchange::IAccount::GetLastCheckoutResetTimeResult result{};
    result.resetTime = 1;

    EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->SetLastCheckoutResetTime(expectedResetTime));
    EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->GetLastCheckoutResetTime(result));
    EXPECT_EQ(expectedResetTime, result.resetTime);
}

TEST_F(Account_L2Test, SetAndGetLastCheckoutResetTime_MaxUint64Boundary)
{
    EXPECT_TRUE(m_AccountPlugin != nullptr);

    if (!m_AccountPlugin)
        return;

    const uint64_t expectedResetTime = std::numeric_limits<uint64_t>::max();
    Exchange::IAccount::GetLastCheckoutResetTimeResult result{};
    result.resetTime = 0;

    EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->SetLastCheckoutResetTime(expectedResetTime));
    EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->GetLastCheckoutResetTime(result));
    EXPECT_EQ(expectedResetTime, result.resetTime);
}

TEST_F(Account_L2Test, SetLastCheckoutResetTime_OverwriteValue)
{
    EXPECT_TRUE(m_AccountPlugin != nullptr);

    if (!m_AccountPlugin)
        return;

    const uint64_t firstResetTime = 111ULL;
    const uint64_t secondResetTime = 222ULL;
    Exchange::IAccount::GetLastCheckoutResetTimeResult result{};
    result.resetTime = 0;

    EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->SetLastCheckoutResetTime(firstResetTime));
    EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->SetLastCheckoutResetTime(secondResetTime));
    EXPECT_EQ(Core::ERROR_NONE, m_AccountPlugin->GetLastCheckoutResetTime(result));
    EXPECT_EQ(secondResetTime, result.resetTime);
}
