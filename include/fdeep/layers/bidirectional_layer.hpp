// Copyright 2016, Tobias Hermann.
// https://github.com/Dobiasd/frugally-deep
// Distributed under the MIT License.
// (See accompanying LICENSE file or at
//  https://opensource.org/licenses/MIT)

#pragma once

#include "fdeep/layers/layer.hpp"
#include "fdeep/recurrent_ops.hpp"

#include <string>
#include <functional>

namespace fdeep
{
namespace internal
{

class bidirectional_layer : public layer
{
public:
    explicit bidirectional_layer(const std::string& name,
                        const std::string& merge_mode,
                        const std::size_t n_units,
                        const std::string& activation,
                        const std::string& recurrent_activation,
                        const std::string& wrapped_layer_type,
                        const bool use_bias,
                        const bool return_sequences,
                        const RowMajorMatrixXf& W_forward,
                        const RowMajorMatrixXf& U_forward,
                        const RowMajorMatrixXf& bias_forward,
                        const RowMajorMatrixXf& W_backward,
                        const RowMajorMatrixXf& U_backward,
                        const RowMajorMatrixXf& bias_backward
                        )
        : layer(name),
        merge_mode_(merge_mode),
        n_units_(n_units),
        activation_(activation),
        recurrent_activation_(recurrent_activation),
        wrapped_layer_type_(wrapped_layer_type),
        use_bias_(use_bias),
        return_sequences_(return_sequences),
        W_forward_(W_forward),
        U_forward_(U_forward),
        bias_forward_(bias_forward),
        W_backward_(W_backward),
        U_backward_(U_backward),
        bias_backward_(bias_backward)
    {
    }
    
protected:
    tensor3s apply_impl(const tensor3s& inputs) const override final
    { 
        tensor3s result_forward = {};
        tensor3s result_backward = {};
        tensor3s bidirectional_result = {};
        
        const tensor3s inputs_reversed = reverse_time_series_in_tensor3s(inputs);
        
        if (wrapped_layer_type_ == "LSTM")
        {
            result_forward = lstm_impl(inputs, n_units_, use_bias_, return_sequences_,
                                                  W_forward_, U_forward_, bias_forward_, activation_, recurrent_activation_);
            result_backward = lstm_impl(inputs_reversed, n_units_, use_bias_, return_sequences_,
                                                   W_backward_, U_backward_, bias_backward_, activation_, recurrent_activation_);
        }
        else
            raise_error("layer '" + wrapped_layer_type_ + "' not yet implemented");
            
        const tensor3s result_backward_reversed = reverse_time_series_in_tensor3s(result_backward);
        
        if (merge_mode_ == "concat")
        {
            for (std::size_t i = 0; i < result_forward.size(); ++i)
                bidirectional_result.push_back(concatenate_tensor3s_depth({result_forward[i], result_backward_reversed[i]}));
        }
        else if (merge_mode_ == "sum")
        {
            for (std::size_t i = 0; i < result_forward.size(); ++i)
                bidirectional_result.push_back(sum_tensor3s({result_forward[i], result_backward_reversed[i]}));
        }
        else if (merge_mode_ == "mul")
        {
            for (std::size_t i = 0; i < result_forward.size(); ++i)
                bidirectional_result.push_back(multiply_tensor3s({result_forward[i], result_backward_reversed[i]}));
        }
        else if (merge_mode_ == "ave")
        {
            for (std::size_t i = 0; i < result_forward.size(); ++i)
                bidirectional_result.push_back(average_tensor3s({result_forward[i], result_backward_reversed[i]}));
        }
        else
            raise_error("merge mode '" + merge_mode_ + "' not valid");
        
        return bidirectional_result;
    }
    
    const std::string merge_mode_;
    const std::size_t n_units_;
    const std::string activation_;
    const std::string recurrent_activation_;
    const std::string wrapped_layer_type_;
    const bool use_bias_;
    const bool return_sequences_;
    const RowMajorMatrixXf W_forward_;
    const RowMajorMatrixXf U_forward_;
    const RowMajorMatrixXf bias_forward_;
    const RowMajorMatrixXf W_backward_;
    const RowMajorMatrixXf U_backward_;
    const RowMajorMatrixXf bias_backward_;
};

} // namespace internal
} // namespace fdeep
