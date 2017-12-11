// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015 Oslo and Akershus University College of Applied Sciences
// and Alfred Bratterud
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <service>
#include <net/inet4>
#include <timers>
#include <net/openssl/server.hpp>
extern http::Response_ptr handle_request(const http::Request&);

void Service::start()
{
  auto& inet = net::Super_stack::get<net::IP4>(0);
  auto* server = new http::OpenSSL_server("/test.pem", "/test.key", inet.tcp());

  server->on_request(
    [] (auto request, auto response_writer) {
      response_writer->set_response(handle_request(*request));
      response_writer->write();
    });

  server->listen(443);

  // Print some useful netstats every 30 secs
  using namespace std::chrono;
  Timers::periodic(5s, 30s,
  [] (uint32_t) {
    auto& inet = net::Super_stack::get<net::IP4>(0);
    printf("<Service> TCP STATUS:\n%s\n", inet.tcp().status().c_str());
  });
}