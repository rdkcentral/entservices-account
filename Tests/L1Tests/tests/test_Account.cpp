/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/


#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "COMLinkMock.h"

#include "Account.h"
#include "AccountImplementation.h"
#include "ServiceMock.h"
#include "Store2Mock.h"
#include "WrapsMock.h"
#include "ThunderPortability.h"
#include <limits>

using ::testing::NiceMock;
using namespace WPEFramework;


class AccountTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::Account> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Context connection;
    Core::JSONRPC::Message message;
    NiceMock<ServiceMock> service;
    NiceMock<COMLinkMock> comLinkMock;
    Core::ProxyType<Plugin::AccountImplementation> AccountImpl;
    string response;
    WrapsImplMock *p_wrapsImplMock   = nullptr;
    ServiceMock  *p_serviceMock  = nullptr;
    Store2Mock  *p_store2Mock  = nullptr;
    AccountTest()
        : plugin(Core::ProxyType<Plugin::Account>::Create())
        , handler(*plugin)
        , connection(1,0,"")
    {
        p_serviceMock = new NiceMock <ServiceMock>;
        p_store2Mock = new NiceMock <Store2Mock>;

        p_wrapsImplMock  = new NiceMock <WrapsImplMock>;
        Wraps::setImpl(p_wrapsImplMock);

        EXPECT_CALL(service, QueryInterfaceByCallsign(::testing::_, ::testing::_))
            .WillOnce(testing::Return(p_store2Mock));

        ON_CALL(comLinkMock, Instantiate(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](const RPC::Object& object, const uint32_t waitTime, uint32_t& connectionId) {
                AccountImpl = Core::ProxyType<Plugin::AccountImplementation>::Create();
                return &AccountImpl;
                }));

        plugin->Initialize(&service);
    }

    virtual ~AccountTest() override
    {

        plugin->Deinitialize(&service);

        if (p_serviceMock != nullptr)
        {
            delete p_serviceMock;
            p_serviceMock = nullptr;
        }
        
        if (p_store2Mock != nullptr)
        {
            delete p_store2Mock;
            p_store2Mock = nullptr;
        }

        Wraps::setImpl(nullptr);
        if (p_wrapsImplMock != nullptr)
        {
            delete p_wrapsImplMock;
            p_wrapsImplMock = nullptr;
        }
    }
};

TEST_F(AccountTest, SetLastCheckoutResetTime_Success)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE,
        handler.Invoke(connection, _T("setLastCheckoutResetTime"), _T("{\"resetTime\":123456789}"), response));
}

TEST_F(AccountTest, SetLastCheckoutResetTime_FailureFromStore)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL,
        handler.Invoke(connection, _T("setLastCheckoutResetTime"), _T("{\"resetTime\":123456789}"), response));
}

TEST_F(AccountTest, SetLastCheckoutResetTime_BoundaryZero)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE,
        handler.Invoke(connection, _T("setLastCheckoutResetTime"), _T("{\"resetTime\":0}"), response));
}

TEST_F(AccountTest, SetLastCheckoutResetTime_BoundaryMaxUint64)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE,
        handler.Invoke(connection, _T("setLastCheckoutResetTime"), _T("{\"resetTime\":18446744073709551615}"), response));
}

TEST_F(AccountTest, SetLastCheckoutResetTime_InvalidPayloadMissingParam)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    // Missing parameter would be passed as 0 due to JSON-COMRPC conversion, so it should succeed.
    EXPECT_EQ(Core::ERROR_NONE,
        handler.Invoke(connection, _T("setLastCheckoutResetTime"), _T("{}"), response));
}

TEST_F(AccountTest, SetLastCheckoutResetTime_InvalidPayloadWrongType)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    // String parameter would be passed as 0 due to JSON-COMRPC conversion, so it should succeed.
    EXPECT_EQ(Core::ERROR_NONE,
        handler.Invoke(connection, _T("setLastCheckoutResetTime"), _T("{\"resetTime\":\"bad\"}"), response));
}

TEST_F(AccountTest, GetLastCheckoutResetTime_DefaultValueWhenNotSet)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::DoAll(
            ::testing::Return(Core::ERROR_NOT_EXIST)));

    EXPECT_EQ(Core::ERROR_NONE,
        handler.Invoke(connection, _T("getLastCheckoutResetTime"), _T("{}"), response));
    EXPECT_TRUE(response.find("\"resetTime\":0") != std::string::npos);
}

TEST_F(AccountTest, GetLastCheckoutResetTime_Success)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::DoAll(
            ::testing::SetArgReferee<3>(std::string("123456789")),
            ::testing::Return(Core::ERROR_NONE)));

    EXPECT_EQ(Core::ERROR_NONE,
        handler.Invoke(connection, _T("getLastCheckoutResetTime"), _T("{}"), response));
    EXPECT_TRUE(response.find("resetTime") != std::string::npos);
}

TEST_F(AccountTest, GetLastCheckoutResetTime_FailureFromStore)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL,
        handler.Invoke(connection, _T("getLastCheckoutResetTime"), _T("{}"), response));
}

TEST_F(AccountTest, GetLastCheckoutResetTime_BoundaryZero)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::DoAll(
            ::testing::SetArgReferee<3>(std::string("0")),
            ::testing::Return(Core::ERROR_NONE)));

    EXPECT_EQ(Core::ERROR_NONE,
        handler.Invoke(connection, _T("getLastCheckoutResetTime"), _T("{}"), response));
    EXPECT_TRUE(response.find("\"resetTime\":0") != std::string::npos);
}

TEST_F(AccountTest, GetLastCheckoutResetTime_BoundaryMaxUint64)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::DoAll(
            ::testing::SetArgReferee<3>(std::string("18446744073709551615")),
            ::testing::Return(Core::ERROR_NONE)));

    EXPECT_EQ(Core::ERROR_NONE,
        handler.Invoke(connection, _T("getLastCheckoutResetTime"), _T("{}"), response));
    EXPECT_TRUE(response.find("18446744073709551615") != std::string::npos);
}

TEST_F(AccountTest, SetLastCheckoutResetTime_UnavailableResource)
{
    Core::ProxyType<Plugin::Account> localPlugin(Core::ProxyType<Plugin::Account>::Create());
    Core::JSONRPC::Handler& localHandler(*localPlugin);
    NiceMock<ServiceMock> localService;
    NiceMock<COMLinkMock> localComLink;
    Core::ProxyType<Plugin::AccountImplementation> localImpl;
    std::string localResponse;

    ON_CALL(localComLink, Instantiate(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](const RPC::Object&, const uint32_t, uint32_t&) {
                localImpl = Core::ProxyType<Plugin::AccountImplementation>::Create();
                return &localImpl;
            }));

    EXPECT_CALL(localService, QueryInterfaceByCallsign(::testing::_, ::testing::_))
        .WillRepeatedly(::testing::Return(static_cast<Store2Mock*>(nullptr)));

    localPlugin->Initialize(&localService);

    EXPECT_EQ(Core::ERROR_UNAVAILABLE,
        localHandler.Invoke(connection, _T("setLastCheckoutResetTime"), _T("{\"resetTime\":99}"), localResponse));

    localPlugin->Deinitialize(&localService);
}

TEST_F(AccountTest, GetLastCheckoutResetTime_UnavailableResource)
{
    Core::ProxyType<Plugin::Account> localPlugin(Core::ProxyType<Plugin::Account>::Create());
    Core::JSONRPC::Handler& localHandler(*localPlugin);
    NiceMock<ServiceMock> localService;
    NiceMock<COMLinkMock> localComLink;
    Core::ProxyType<Plugin::AccountImplementation> localImpl;
    std::string localResponse;

    ON_CALL(localComLink, Instantiate(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](const RPC::Object&, const uint32_t, uint32_t&) {
                localImpl = Core::ProxyType<Plugin::AccountImplementation>::Create();
                return &localImpl;
            }));

    EXPECT_CALL(localService, QueryInterfaceByCallsign(::testing::_, ::testing::_))
        .WillRepeatedly(::testing::Return(static_cast<Store2Mock*>(nullptr)));

    localPlugin->Initialize(&localService);

    EXPECT_EQ(Core::ERROR_UNAVAILABLE,
        localHandler.Invoke(connection, _T("getLastCheckoutResetTime"), _T("{}"), localResponse));

    localPlugin->Deinitialize(&localService);
}

