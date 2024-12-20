/* Copyright 2022 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include "tensorflow/compiler/jit/xla_compile_util.h"

#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "tensorflow/core/framework/fake_input.h"
#include "tensorflow/core/kernels/ops_testutil.h"

namespace tensorflow {
namespace {

using ::testing::ElementsAreArray;

TEST_F(OpsTestBase, Basic) {
  TF_EXPECT_OK(NodeDefBuilder("identity_op", "Identity")
                   .Input(FakeInput(DT_FLOAT))
                   .Attr("T", DT_FLOAT)
                   .Finalize(node_def()));
  TF_EXPECT_OK(InitOp());
  AddInputFromArray<float>(TensorShape({1, 2}), {0, 1});
  TF_EXPECT_OK(RunOpKernel());

  auto arg = BuildSingleOpCompileArgument(context_.get());

  EXPECT_THAT(arg.output_dtypes, ElementsAreArray({DT_FLOAT}));
  EXPECT_EQ(arg.node_def.SerializeAsString(),
            context_->op_kernel().def().SerializeAsString());
  EXPECT_EQ(arg.config_proto.ByteSizeLong(), 0);
}

TEST(GetExecutableOptionTest, Basic) {
  XlaCompiler::Options options;
  options.device_ordinal = 0;
  options.alias_passthrough_params = true;
  options.detailed_logging = true;
  XlaCompiler::CompilationResult result;
  xla::Shape xla_output_shape;
  result.xla_output_shape = xla_output_shape;

  auto build_option =
      GetExecutableBuildOptions(options, result, /*default_device_ordinal=*/-1);

  EXPECT_EQ(build_option.device_ordinal(), 0);
  EXPECT_EQ(build_option.result_layout()->ToString(),
            xla_output_shape.ToString());
  EXPECT_EQ(build_option.alias_passthrough_params(), true);
  EXPECT_EQ(build_option.debug_options().xla_detailed_logging_and_dumping(),
            true);
  LOG(ERROR) << build_option.ToString();
}

TEST(GetExecutableOptionTest, DefaultDeviceOrdinal) {
  XlaCompiler::Options options;
  XlaCompiler::CompilationResult result;

  auto build_option =
      GetExecutableBuildOptions(options, result, /*default_device_ordinal=*/0);

  EXPECT_EQ(build_option.device_ordinal(), 0);
}

TEST(GetExecutableOptionTest, DeviceOrdinalNotSet) {
  XlaCompiler::Options options;
  XlaCompiler::CompilationResult result;

  auto build_option =
      GetExecutableBuildOptions(options, result, /*default_device_ordinal=*/-1);

  EXPECT_EQ(build_option.device_ordinal(), -1);
}

}  // namespace
}  // namespace tensorflow
