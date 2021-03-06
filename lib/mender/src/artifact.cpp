// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2016-2017 Oslo and Akershus University College of Applied Sciences
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

#include <mender/artifact.hpp>
#include <rapidjson/document.h>

namespace mender {

  Artifact::Artifact(byte_seq data)
    : data_(std::move(data)),
      reader_{},
      artifact_{}
  {
    parse();
  }

  void Artifact::parse()
  {
    using namespace tar;

    // Print and add

    MENDER_INFO("Artifact", "Parsing data as mender arifact (%u bytes)", (uint32_t)data_.size());
    artifact_ = reader_.read_uncompressed(data_.data(), data_.size());

    auto& elements = artifact_.elements();
    MENDER_INFO("Artifact", "Content");

    // Version
    auto& version_element = elements.at(VERSION);
    parse_version(version_element.content());
    MENDER_INFO2("%s", version_element.name().c_str());

    // Header
    auto& header_element = elements.at(HEADER);
    MENDER_INFO2("%s", header_element.name().c_str());
    header_ = reader_.decompress(header_element);

    auto& header_elements = header_.elements();

    for (size_t i = 0; i < header_.elements().size(); i++) {
      const std::string header_element_name = header_.elements().at(i).name();
      MENDER_INFO2("\t%s", header_element_name.c_str());
    }

    // header_info
    auto& header_info = header_elements.at(0);
    parse_header_info(header_info.content());

    // Data
    for (size_t i = DATA_START; i < elements.size(); i++) {
      auto& data_tar_gz = elements.at(i);
      const std::string data_name = data_tar_gz.name();

      assert(data_name.find("data/") != std::string::npos and data_tar_gz.is_tar_gz());

      MENDER_INFO2("%s", data_tar_gz.name().c_str());

      auto tar = reader_.decompress(data_tar_gz);

      for (auto& data_element : tar.elements())
      {
        (void) data_element;
        MENDER_INFO2("\t%s", data_element.name().c_str());
      }

      updates_.push_back(tar);
    }
  }

  void Artifact::parse_version(const uint8_t* version)
  {
    using namespace rapidjson;
    Document d;
    d.Parse((const char*)version);
    format_ = d["format"].GetString();
    version_ = d["version"].GetInt();
  }

  void Artifact::parse_header_info(const uint8_t* header_info)
  {
    using namespace rapidjson;
    Document d;
    d.Parse((const char*)header_info);
    name_ = d["artifact_name"].GetString();
  }
}

