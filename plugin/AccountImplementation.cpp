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

#include "AccountImplementation.h"

#include "UtilsLogging.h"

#define ACCOUNT_NAMESPACE "org.rdk.Account"
#define ACCOUNT_LAST_CHECKOUT_RESET_TIME_KEY "LastCheckoutResetTime"

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(AccountImplementation, 1, 0);
    AccountImplementation* AccountImplementation::_instance = nullptr;
    
    AccountImplementation::AccountImplementation()
    : _adminLock()
    , _service(nullptr)
    {
        LOGINFO("Create AccountImplementation Instance");
        AccountImplementation::_instance = this;
    }

    AccountImplementation::~AccountImplementation()
    {

        LOGINFO("Call AccountImplementation destructor\n");

        AccountImplementation::_instance = nullptr;

        if (_store != nullptr) {
            _store->Release();
            _store = nullptr;
        }

        if (_service != nullptr) {
            _service->Release();
            _service = nullptr;
        }
    }
    uint32_t AccountImplementation::Configure(PluginHost::IShell* service)
    {
        LOGINFO("Configuring AccountImplementation");
        uint32_t status = Core::ERROR_NONE;
        ASSERT(service != nullptr);
        _service = service;
        _service->AddRef();

        _store = _service->QueryInterfaceByCallsign<Exchange::IStore2>(_T("org.rdk.PersistentStore"));
        if (_store == nullptr) {
            LOGERR("IStore2 interface is not available");
            status = Core::ERROR_UNAVAILABLE;
        }

        return status;
    }

    Core::hresult AccountImplementation::GetLastCheckoutResetTime(uint64_t &resetTime /* @out */) const
    {
        if (_store == nullptr) {
            LOGERR("IStore2 interface is not available");
            return Core::ERROR_UNAVAILABLE;
        }

        std::string resetTimeStr;
        uint32_t ttl;
        Core::hresult result = _store->GetValue(Exchange::IStore2::ScopeType::DEVICE, ACCOUNT_NAMESPACE, ACCOUNT_LAST_CHECKOUT_RESET_TIME_KEY, resetTimeStr, ttl);

        if (result == Core::ERROR_NONE) {
            resetTime = std::stoull(resetTimeStr);
        }

        return result;
    }

    Core::hresult AccountImplementation::SetLastCheckoutResetTime(const uint64_t resetTime)
    {
        if (_store == nullptr) {
            LOGERR("IStore2 interface is not available");
            return Core::ERROR_UNAVAILABLE;
        }

        std::string resetTimeStr = std::to_string(resetTime);
        Core::hresult result =_store->SetValue(Exchange::IStore2::ScopeType::DEVICE, ACCOUNT_NAMESPACE, ACCOUNT_LAST_CHECKOUT_RESET_TIME_KEY, resetTimeStr, 0);
        
        return result;
    }

} // namespace Plugin
} // namespace WPEFramework
