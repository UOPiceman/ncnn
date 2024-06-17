// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2021 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include "pass_level2.h"

namespace pnnx {

class F_conv1d : public GraphRewriterPass
{
public:
    const char* match_pattern_graph() const
    {
        return R"PNNXIR(7767517
16 15
pnnx.Input              input_0     0 1 input
pnnx.Input              input_1     0 1 weight
pnnx.Input              input_2     0 1 bias
pnnx.Input              input_3     0 1 stride
pnnx.Input              input_4     0 1 padding
pnnx.Input              input_5     0 1 dilation
pnnx.Input              input_6     0 1 groups
prim::Constant          op_0        0 1 transposed value=False
prim::Constant          op_1        0 1 output_padding_w value=0
prim::ListConstruct     op_2        1 1 output_padding_w output_padding
prim::Constant          op_3        0 1 benchmark value=*
prim::Constant          op_4        0 1 deterministic value=*
prim::Constant          op_5        0 1 cudnn_enabled value=*
prim::Constant          op_6        0 1 allow_tf32 value=*
aten::_convolution      op_7        13 1 input weight bias stride padding dilation transposed output_padding groups benchmark deterministic cudnn_enabled allow_tf32 out
pnnx.Output             output      1 0 out
)PNNXIR";
    }

    const char* type_str() const
    {
        return "F.conv1d";
    }
};

REGISTER_GLOBAL_PNNX_GRAPH_REWRITER_PASS(F_conv1d, 10)

class F_convmode : public GraphRewriterPass
{
public:
    const char* match_pattern_graph() const
    {
        return R"PNNXIR(7767517
9 8
pnnx.Input              input_0     0 1 input
pnnx.Input              input_1     0 1 weight
pnnx.Input              input_2     0 1 bias
pnnx.Input              input_3     0 1 stride
pnnx.Input              input_4     0 1 padding
pnnx.Input              input_5     0 1 dilation
pnnx.Input              input_6     0 1 groups
aten::_convolution_mode op_0        7 1 input weight bias stride padding dilation groups out
pnnx.Output             output      1 0 out
)PNNXIR";
    }

    const char* type_str() const
    {
        return "F.convmode";
    }
};

REGISTER_GLOBAL_PNNX_GRAPH_REWRITER_PASS(F_convmode, 10)

class F_conv1d_onnx : public GraphRewriterPass
{
public:
    const char* match_pattern_graph() const
    {
        return R"PNNXIR(7767517
5 4
pnnx.Input              input_0     0 1 input
pnnx.Input              input_1     0 1 weight
pnnx.Input              input_2     0 1 bias
Conv                    op_0        3 1 input weight bias out kernel_shape=%kernel_shape strides=%strides pads=%pads dilations=%dilations group=%group
pnnx.Output             output      1 0 out
)PNNXIR";
    }

    const char* type_str() const
    {
        return "F.conv1d";
    }

    bool match(const std::map<std::string, Parameter>& captured_params) const
    {
        if (captured_params.at("kernel_shape").type != 5)
            return false;

        if (captured_params.at("kernel_shape").ai.size() != 1)
            return false;

        if (captured_params.at("strides").type != 5)
            return false;

        if (captured_params.at("strides").ai.size() != 1)
            return false;

        if (captured_params.at("dilations").type != 5)
            return false;

        if (captured_params.at("dilations").ai.size() != 1)
            return false;

        if (captured_params.at("group").type != 2)
            return false;

        if (captured_params.at("pads").type != 5)
            return false;

        const std::vector<int>& pads = captured_params.at("pads").ai;
        if (pads.size() != 2 || pads[0] != pads[1])
            return false;

        return true;
    }

    void write(Operator* op, const std::map<std::string, Parameter>& captured_params) const
    {
        const std::vector<int>& pads = captured_params.at("pads").ai;

        op->params["stride"] = captured_params.at("strides");
        op->params["dilation"] = captured_params.at("dilations");
        op->params["groups"] = captured_params.at("group");
        op->params["padding"] = {pads[0]};
    }
};

REGISTER_GLOBAL_PNNX_GRAPH_REWRITER_PASS(F_conv1d_onnx, 10)

class F_conv1d_onnx_0 : public F_conv1d_onnx
{
public:
    const char* match_pattern_graph() const
    {
        return R"PNNXIR(7767517
4 3
pnnx.Input              input_0     0 1 input
pnnx.Input              input_1     0 1 weight
Conv                    op_0        2 1 input weight out kernel_shape=%kernel_shape strides=%strides pads=%pads dilations=%dilations group=%group
pnnx.Output             output      1 0 out
)PNNXIR";
    }

    const char* replace_pattern_graph() const
    {
        return R"PNNXIR(7767517
4 3
pnnx.Input              input_0     0 1 input
pnnx.Input              input_1     0 1 weight
F.conv1d                conv        2 1 input weight out bias=None
pnnx.Output             output      1 0 out
)PNNXIR";
    }
};

REGISTER_GLOBAL_PNNX_GRAPH_REWRITER_PASS(F_conv1d_onnx_0, 10)

class F_conv1d_onnx_1 : public GraphRewriterPass
{
public:
    const char* match_pattern_graph() const
    {
        return R"PNNXIR(7767517
5 4
pnnx.Input              input_0     0 1 input
pnnx.Input              input_1     0 1 weight
pnnx.Input              input_2     0 1 bias
Conv                    op_0        3 1 input weight bias out strides=%strides pads=%pads dilations=%dilations group=%group auto_pad=NOTSET
pnnx.Output             output      1 0 out
)PNNXIR";
    }

    const char* type_str() const
    {
        return "F.conv1d";
    }

    bool match(const std::map<std::string, Parameter>& captured_params) const
    {
        if (captured_params.at("strides").type != 5)
            return false;

        if (captured_params.at("strides").ai.size() != 1)
            return false;

        if (captured_params.at("dilations").type != 5)
            return false;

        if (captured_params.at("dilations").ai.size() != 1)
            return false;

        if (captured_params.at("group").type != 2)
            return false;

        if (captured_params.at("pads").type != 5)
            return false;

        const std::vector<int>& pads = captured_params.at("pads").ai;
        if (pads.size() != 2 || pads[0] != pads[1])
            return false;

        return true;
    }

    void write(Operator* op, const std::map<std::string, Parameter>& captured_params) const
    {
        const std::vector<int>& pads = captured_params.at("pads").ai;

        op->params["stride"] = captured_params.at("strides");
        op->params["dilation"] = captured_params.at("dilations");
        op->params["groups"] = captured_params.at("group");
        op->params["padding"] = {pads[0]};
    }
};

REGISTER_GLOBAL_PNNX_GRAPH_REWRITER_PASS(F_conv1d_onnx_1, 10)

} // namespace pnnx
