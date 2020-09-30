// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "generator/internal/codegen_utils.h"
#include "google/cloud/internal/absl_str_cat_quiet.h"
#include "google/cloud/internal/absl_str_join_quiet.h"
#include "google/cloud/internal/absl_str_replace_quiet.h"
#include "absl/strings/str_split.h"
#include <google/protobuf/compiler/code_generator.h>
#include <cctype>
#include <string>

namespace google {
namespace cloud {
namespace generator_internal {

std::string GeneratedFileSuffix() { return ".gcpcxx.pb"; }

std::string LocalInclude(absl::string_view header) {
  return absl::StrCat("#include \"", header, "\"\n");
}

std::string SystemInclude(absl::string_view header) {
  return absl::StrCat("#include <", header, ">\n");
}

std::string CamelCaseToSnakeCase(absl::string_view input) {
  std::string output;
  for (auto i = 0U; i < input.size(); ++i) {
    auto const uc = static_cast<unsigned char>(input[i]);
    auto const lower = static_cast<char>(std::tolower(uc));
    if (input[i] != '_' && i + 2 < input.size()) {
      if (std::isupper(static_cast<unsigned char>(input[i + 1])) &&
          std::islower(static_cast<unsigned char>(input[i + 2]))) {
        output += lower;
        output += '_';
        continue;
      }
    }
    if (input[i] != '_' && i + 1 < input.size()) {
      if ((std::islower(static_cast<unsigned char>(input[i])) ||
           std::isdigit(static_cast<unsigned char>(input[i]))) &&
          std::isupper(static_cast<unsigned char>(input[i + 1]))) {
        output += lower;
        output += '_';
        continue;
      }
    }
    output += lower;
  }
  return output;
}

std::string ServiceNameToFilePath(absl::string_view service_name) {
  std::vector<absl::string_view> components = absl::StrSplit(service_name, ".");
  absl::ConsumeSuffix(&components.back(), "Service");
  auto formatter = [](std::string* s, absl::string_view sv) {
    *s += CamelCaseToSnakeCase(sv);
  };
  return absl::StrJoin(components, "/", formatter);
}

std::string ProtoNameToCppName(absl::string_view proto_name) {
  return "::" + absl::StrReplaceAll(proto_name, {{".", "::"}});
}

std::vector<std::string> BuildNamespaces(std::string const& product_path,
                                         NamespaceType ns_type) {
  std::vector<std::string> v =
      absl::StrSplit(product_path, '/', absl::SkipEmpty());
  std::string name =
      absl::StrJoin(v.begin() + (v.size() > 2 ? 2 : 0), v.end(), "_");
  std::string inline_ns = absl::AsciiStrToUpper(name) + "_CLIENT_NS";
  if (ns_type == NamespaceType::kInternal) {
    absl::StrAppend(&name, "_internal");
  }

  return std::vector<std::string>{"google", "cloud", name, inline_ns};
}

StatusOr<std::vector<std::pair<std::string, std::string>>>
ProcessCommandLineArgs(std::string const& parameters) {
  std::vector<std::pair<std::string, std::string>> command_line_args;
  google::protobuf::compiler::ParseGeneratorParameter(parameters,
                                                      &command_line_args);

  auto product_path =
      std::find_if(command_line_args.begin(), command_line_args.end(),
                   [](std::pair<std::string, std::string> const& p) {
                     return p.first == "product_path";
                   });
  if (product_path == command_line_args.end() || product_path->second.empty()) {
    return Status(StatusCode::kInvalidArgument,
                  "--cpp_codegen_opt=product_path=<path> must be specified.");
  }

  auto& path = product_path->second;
  if (path.front() == '/') {
    path = path.substr(1);
  }
  if (path.back() != '/') {
    path += '/';
  }
  return command_line_args;
}

std::string CopyrightLicenseFileHeader() {
  static auto constexpr kHeader =  // clang-format off
  "// Copyright 2020 Google LLC\n"
  "//\n"
  "// Licensed under the Apache License, Version 2.0 (the \"License\");\n"
  "// you may not use this file except in compliance with the License.\n"
  "// You may obtain a copy of the License at\n"
  "//\n"
  "//      https://www.apache.org/licenses/LICENSE-2.0\n"
  "//\n"
  "// Unless required by applicable law or agreed to in writing, software\n"
  "// distributed under the License is distributed on an \"AS IS\" BASIS,\n"
  "// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
  "// See the License for the specific language governing permissions and\n"
  "// limitations under the License.\n\n";
  // clang-format on
  return kHeader;
}

}  // namespace generator_internal
}  // namespace cloud
}  // namespace google
